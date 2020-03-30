// Copyright (c) 2011-2020 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#if defined(HAVE_CONFIG_H)

#endif

#include <bitcoin.h>
#include <bitcoingui.h>
#include <iotaqtconfig.h>
#include <clientmodel.h>
#include <guiconstants.h>
#include <guiutil.h>
#include <intro.h>
#include <networkstyle.h>
#include <optionsmodel.h>
#include <platformstyle.h>
#include <splashscreen.h>
#include <utilitydialog.h>
#include <tinyformat.h>
#include <winshutdownmonitor.h>

#include <walletcontroller.h>
#include <walletmodel.h>


#include <interfaces/handler.h>
#include <interfaces/node.h>

#include <uint256.h>
#include <util/threadnames.h>

#include <memory>

#include <QApplication>
#include <QDebug>
#include <QLibraryInfo>
#include <QLocale>
#include <QMessageBox>
#include <QSettings>
#include <QThread>
#include <QTimer>
#include <QTranslator>
#include <QLockFile>
#include <QDir>

#if defined(QT_STATICPLUGIN)
#include <QtPlugin>
#if defined(QT_QPA_PLATFORM_XCB)
Q_IMPORT_PLUGIN(QXcbIntegrationPlugin);
#elif defined(QT_QPA_PLATFORM_WINDOWS)
Q_IMPORT_PLUGIN(QWindowsIntegrationPlugin);
#elif defined(QT_QPA_PLATFORM_COCOA)
Q_IMPORT_PLUGIN(QCocoaIntegrationPlugin);
#endif
#endif

// Declare meta types used for QMetaObject::invokeMethod
Q_DECLARE_METATYPE(bool*)
Q_DECLARE_METATYPE(CAmount)
Q_DECLARE_METATYPE(uint256)

static QString GetLangTerritory()
{
    QSettings settings;
    // Get desired locale (e.g. "de_DE")
    // 1) System default language
    QString lang_territory = QLocale::system().name();
    // 2) Language from QSettings
    QString lang_territory_qsettings = settings.value("language", "").toString();
    if(!lang_territory_qsettings.isEmpty())
        lang_territory = lang_territory_qsettings;

    return lang_territory;
}

/** Set up translations */
static void initTranslations(QTranslator &qtTranslatorBase, QTranslator &qtTranslator, QTranslator &translatorBase, QTranslator &translator)
{
    // Remove old translators
    QApplication::removeTranslator(&qtTranslatorBase);
    QApplication::removeTranslator(&qtTranslator);
    QApplication::removeTranslator(&translatorBase);
    QApplication::removeTranslator(&translator);

    // Get desired locale (e.g. "de_DE")
    // 1) System default language
    QString lang_territory = GetLangTerritory();

    // Convert to "de" only by truncating "_DE"
    QString lang = lang_territory;
    lang.truncate(lang_territory.lastIndexOf('_'));

    // Load language files for configured locale:
    // - First load the translator for the base language, without territory
    // - Then load the more specific locale translator

    // Load e.g. qt_de.qm
    if (qtTranslatorBase.load("qt_" + lang, QLibraryInfo::location(QLibraryInfo::TranslationsPath)))
        QApplication::installTranslator(&qtTranslatorBase);

    // Load e.g. qt_de_DE.qm
    if (qtTranslator.load("qt_" + lang_territory, QLibraryInfo::location(QLibraryInfo::TranslationsPath)))
        QApplication::installTranslator(&qtTranslator);

    // Load e.g. bitcoin_de.qm (shortcut "de" needs to be defined in bitcoin.qrc)
    if (translatorBase.load(lang, ":/translations/"))
        QApplication::installTranslator(&translatorBase);

    // Load e.g. bitcoin_de_DE.qm (shortcut "de_DE" needs to be defined in bitcoin.qrc)
    if (translator.load(lang_territory, ":/translations/"))
        QApplication::installTranslator(&translator);
}

/* qDebug() message handler --> debug.log */
//void DebugMessageHandler(QtMsgType type, const QMessageLogContext& context, const QString &msg)
//{
//    Q_UNUSED(context);
//    qDebug("GUI: %s\n", msg.toUtf8().data());
//}

BitcoinCore::BitcoinCore(interfaces::Node& node) :
    QObject(), m_node(node)
{
}

void BitcoinCore::handleRunawayException(const std::exception *e)
{
    //    PrintExceptionContinue(e, "Runaway exception");
    Q_EMIT runawayException(QString::fromStdString(m_node.getWarnings()));
}

void BitcoinCore::initialize()
{
    try
    {
        qDebug() << __func__ << ": Running initialization in thread";
        util::ThreadRename("qt-init");
        bool rv = m_node.appInitMain();
        Q_EMIT initializeResult(rv);
    } catch (const std::exception& e) {
        handleRunawayException(&e);
    } catch (...) {
        handleRunawayException(nullptr);
    }
}

void BitcoinCore::shutdown()
{
    try
    {
        qDebug() << __func__ << ": Running Shutdown in thread";
        m_node.appShutdown();
        qDebug() << __func__ << ": Shutdown finished";
        Q_EMIT shutdownResult();
    } catch (const std::exception& e) {
        handleRunawayException(&e);
    } catch (...) {
        handleRunawayException(nullptr);
    }
}

static int qt_argc = 1;
static const char* qt_argv = "iota-qt";

BitcoinApplication::BitcoinApplication(interfaces::Node& node):
    QApplication(qt_argc, const_cast<char **>(&qt_argv)),
    coreThread(nullptr),
    m_node(node),
    optionsModel(nullptr),
    clientModel(nullptr),
    window(nullptr),
    pollShutdownTimer(nullptr),
    returnValue(0),
    platformStyle(nullptr)
{
    setQuitOnLastWindowClosed(false);
}

void BitcoinApplication::setupPlatformStyle()
{
    // UI per-platform customization
    // This must be done inside the BitcoinApplication constructor, or after it, because
    // PlatformStyle::instantiate requires a QApplication
    std::string platformName;
    platformName = BitcoinGUI::DEFAULT_UIPLATFORM;
    platformStyle = PlatformStyle::instantiate(QString::fromStdString(platformName));
    if (!platformStyle) // Fall back to "other" if specified name not found
        platformStyle = PlatformStyle::instantiate("other");
    assert(platformStyle);
}

BitcoinApplication::~BitcoinApplication()
{
    if(coreThread)
    {
        qDebug() << __func__ << ": Stopping thread";
        coreThread->quit();
        coreThread->wait();
        qDebug() << __func__ << ": Stopped thread";
    }

    delete window;
    window = nullptr;
    delete optionsModel;
    optionsModel = nullptr;
    delete platformStyle;
    platformStyle = nullptr;
}



void BitcoinApplication::createOptionsModel(bool resetSettings)
{
    optionsModel = new OptionsModel(m_node, nullptr, resetSettings);
}

void BitcoinApplication::createWindow(const NetworkStyle *networkStyle)
{
    window = new BitcoinGUI(m_node, platformStyle, networkStyle, nullptr);

    pollShutdownTimer = new QTimer(window);
    connect(pollShutdownTimer, &QTimer::timeout, window, &BitcoinGUI::detectShutdown);
}

void BitcoinApplication::createSplashScreen(const NetworkStyle *networkStyle)
{
    SplashScreen *splash = new SplashScreen(m_node, nullptr, networkStyle);
    // We don't hold a direct pointer to the splash screen after creation, but the splash
    // screen will take care of deleting itself when finish() happens.
    splash->show();
    connect(this, &BitcoinApplication::splashFinished, splash, &SplashScreen::finish);
    connect(this, &BitcoinApplication::requestedShutdown, splash, &QWidget::close);
}

bool BitcoinApplication::baseInitialize()
{
    return m_node.baseInitialize();
}

void BitcoinApplication::startThread()
{
    if(coreThread)
        return;
    coreThread = new QThread(this);
    BitcoinCore *executor = new BitcoinCore(m_node);
    executor->moveToThread(coreThread);

    /*  communication to and from thread */


    connect(executor, &BitcoinCore::initializeResult, this, &BitcoinApplication::initializeResult);
    connect(executor, &BitcoinCore::shutdownResult, this, &BitcoinApplication::shutdownResult);
    connect(executor, &BitcoinCore::runawayException, this, &BitcoinApplication::handleRunawayException);
    connect(this, &BitcoinApplication::requestedInitialize, executor, &BitcoinCore::initialize);
    connect(this, &BitcoinApplication::requestedShutdown, executor, &BitcoinCore::shutdown);
    /*  make sure executor object is deleted in its own thread */
    connect(coreThread, &QThread::finished, executor, &QObject::deleteLater);

    coreThread->start();
}

void BitcoinApplication::parameterSetup()
{
    // Default printtoconsole to false for the GUI. GUI programs should not
    // print to the console unnecessarily.

    m_node.initLogging();
    m_node.initParameterInteraction();
}

void BitcoinApplication::InitializePruneSetting(bool prune)
{
    // If prune is set, intentionally override existing prune size with
    // the default size since this is called when choosing a new datadir.
    optionsModel->SetPruneTargetGB(prune ? DEFAULT_PRUNE_TARGET_GB : 0, true);
}

void BitcoinApplication::requestInitialize()
{
    qDebug() << __func__ << ": Requesting initialize";
    startThread();
    Q_EMIT requestedInitialize();
}

void BitcoinApplication::requestShutdown()
{
    // Show a simple window indicating shutdown status
    // Do this first as some of the steps may take some time below,
    // for example the RPC console may still be executing a command.
    shutdownWindow.reset(ShutdownWindow::showShutdownWindow(window));

    qDebug() << __func__ << ": Requesting shutdown";
    startThread();
    window->hide();
    // Must disconnect node signals otherwise current thread can deadlock since
    // no event loop is running.
    window->unsubscribeFromCoreSignals();
    // Request node shutdown, which can interrupt long operations, like
    // rescanning a wallet.
    m_node.startShutdown();
    // Unsetting the client model can cause the current thread to wait for node
    // to complete an operation, like wait for a RPC execution to complete.
    window->setClientModel(nullptr);
    pollShutdownTimer->stop();

    delete clientModel;
    clientModel = nullptr;

    // Request shutdown from core thread
    Q_EMIT requestedShutdown();
}

void BitcoinApplication::initializeResult(bool success)
{
    qDebug() << __func__ << ": Initialization result: " << success;
    // Set exit result.
    returnValue = success ? EXIT_SUCCESS : EXIT_FAILURE;
    if(success)
    {
        // Log this only after AppInitMain finishes, as then logging setup is guaranteed complete
        qInfo() << "Platform customization:" << platformStyle->getName();
        clientModel = new ClientModel(m_node, optionsModel);
        window->setClientModel(clientModel);
        m_wallet_controller = new WalletController(m_node, platformStyle, optionsModel, this);
        window->setWalletController(m_wallet_controller);


        // If -min option passed, start window minimized (iconified) or minimized to tray
        window->show();
        Q_EMIT splashFinished();
        Q_EMIT windowShown(window);

        // Now that initialization/startup is done, process any command-line
        // bitcoin: URIs or payment requests:

        pollShutdownTimer->start(200);
    } else {
        Q_EMIT splashFinished(); // Make sure splash screen doesn't stick around during shutdown
        quit(); // Exit first main loop invocation
    }
}

void BitcoinApplication::shutdownResult()
{
    quit(); // Exit second main loop invocation after shutdown finished
}

void BitcoinApplication::handleRunawayException(const QString &message)
{
    QMessageBox::critical(nullptr, "Runaway exception", BitcoinGUI::tr("A fatal error occurred. Iota-Qt can no longer continue safely and will quit.") + QString("\n\n") + message);
    ::exit(EXIT_FAILURE);
}

WId BitcoinApplication::getMainWinId() const
{
    if (!window)
        return 0;

    return window->winId();
}

int GuiMain(int argc, char* argv[])
{
    util::ThreadSetInternalName("main");

    std::unique_ptr<interfaces::Node> node = interfaces::MakeNode();
    QDir dataDir(QString::fromStdString(node->getDataDir()));
    QLockFile lockFile(dataDir.absoluteFilePath("lock.lock"));
    lockFile.setStaleLockTime(0);
    if (!lockFile.tryLock(1000)) {
        qDebug() << "Application is already running";
        return -1;
    }

    // Do not refer to data directory yet, this can be overridden by Intro::pickDataDirectory

    /// 1. Basic Qt initialization (not dependent on parameters or configuration)
    //    Q_INIT_RESOURCE(bitcoin);
    //    Q_INIT_RESOURCE(bitcoin_locale);

    // Generate high-dpi pixmaps
    QApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
#if QT_VERSION >= 0x050600
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
#endif

    BitcoinApplication app(*node);

    // Register meta types used for QMetaObject::invokeMethod and Qt::QueuedConnection
    qRegisterMetaType<bool*>();
    qRegisterMetaType<WalletModel*>();
    // Register typedefs (see http://qt-project.org/doc/qt-5/qmetatype.html#qRegisterMetaType)
    // IMPORTANT: if CAmount is no longer a typedef use the normal variant above (see https://doc.qt.io/qt-5/qmetatype.html#qRegisterMetaType-1)
    qRegisterMetaType<CAmount>("CAmount");
    qRegisterMetaType<size_t>("size_t");

    qRegisterMetaType<std::function<void()>>("std::function<void()>");
    qRegisterMetaType<QMessageBox::Icon>("QMessageBox::Icon");

    /// 2. Parse command-line options. We do this after qt in order to show an error if there are problems parsing these
    // Command-line options take precedence:
    node->setupServerArgs();
    //    SetupUIArgs();
    std::string error;
    if (!node->parseParameters(argc, argv, error)) {
        node->initError(strprintf("Error parsing command line arguments: %s\n", error));
        // Create a message box, because the gui has neither been created nor has subscribed to core signals
        QMessageBox::critical(nullptr, PACKAGE_NAME,
                              // message can not be translated because translations have not been initialized
                              QString::fromStdString("Error parsing command line arguments: %1.").arg(QString::fromStdString(error)));
        return EXIT_FAILURE;
    }

    // Now that the QApplication is setup and we have parsed our parameters, we can set the platform style
    app.setupPlatformStyle();

    /// 3. Application identification
    // must be set before OptionsModel is initialized or translations are loaded,
    // as it is used to locate QSettings
    QApplication::setOrganizationName(QAPP_ORG_NAME);
    QApplication::setOrganizationDomain(QAPP_ORG_DOMAIN);
    QApplication::setApplicationName(QAPP_APP_NAME_DEFAULT);

    /// 4. Initialization of translations, so that intro dialog is in user's language
    // Now that QSettings are accessible, initialize translations
    QTranslator qtTranslatorBase, qtTranslator, translatorBase, translator;
    initTranslations(qtTranslatorBase, qtTranslator, translatorBase, translator);

    // Show help message immediately after parsing command-line options (for "-lang") and setting locale,
    // but before showing splash screen.
    //    if (HelpRequested(gArgs) || gArgs.IsArgSet("-version")) {
    //        HelpMessageDialog help(*node, nullptr, gArgs.IsArgSet("-version"));
    //        help.showOrPrint();
    //        return EXIT_SUCCESS;
    //    }

    /// 5. Now that settings and translations are available, ask user for data directory
    // User language is set up: pick a data directory
    bool did_show_intro = false;
    bool prune = false; // Intro dialog prune check box
    // Gracefully exit if the user cancels
    if (!Intro::showIfNeeded(*node, did_show_intro, prune)) return EXIT_SUCCESS;

    /// 6. Determine availability of data directory and parse bitcoin.conf
    /// - Do not call GetDataDir(true) before this step finishes
    //    if (!CheckDataDirOption()) {
    //        node->initError(strprintf("Specified data directory \"%s\" does not exist.\n", gArgs.GetArg("-datadir", "")));
    //        QMessageBox::critical(nullptr, PACKAGE_NAME,
    //            QObject::tr("Error: Specified data directory \"%1\" does not exist.").arg(QString::fromStdString(gArgs.GetArg("-datadir", ""))));
    //        return EXIT_FAILURE;
    //    }

    if (!node->readConfigFiles(error)) {
        node->initError(strprintf("Error reading configuration file: %s\n", error));
        QMessageBox::critical(nullptr, PACKAGE_NAME,
                              QObject::tr("Error: Cannot parse configuration file: %1.").arg(QString::fromStdString(error)));
        return EXIT_FAILURE;
    }

    /// 7. Determine network (and switch to network specific options)
    // - Do not call Params() before this step
    // - Do this after parsing the configuration file, as the network can be switched there
    // - QSettings() will use the new application name after this, resulting in network-specific settings
    // - Needs to be done before createOptionsModel

    // Check for -chain, -testnet or -regtest parameter (Params() calls are only valid after this clause)
    try {
        node->selectParams("mainnet");
    } catch(std::exception &e) {
        node->initError(strprintf("%s\n", e.what()));
        QMessageBox::critical(nullptr, PACKAGE_NAME, QObject::tr("Error: %1").arg(e.what()));
        return EXIT_FAILURE;
    }

    QScopedPointer<const NetworkStyle> networkStyle(NetworkStyle::instantiate("main"));
    assert(!networkStyle.isNull());
    // Allow for separate UI settings for testnets
    QApplication::setApplicationName(networkStyle->getAppName());
    // Re-initialize translations after changing application name (language in network-specific settings can be different)
    initTranslations(qtTranslatorBase, qtTranslator, translatorBase, translator);

    /// 9. Main GUI initialization
    // Install global event filter that makes sure that long tooltips can be word-wrapped
    app.installEventFilter(new GUIUtil::ToolTipToRichTextFilter(TOOLTIP_WRAP_THRESHOLD, &app));
#if defined(Q_OS_WIN)
    // Install global event filter for processing Windows session related Windows messages (WM_QUERYENDSESSION and WM_ENDSESSION)
    qApp->installNativeEventFilter(new WinShutdownMonitor());
#endif
    // Install qDebug() message handler to route to debug.log
    //    qInstallMessageHandler(DebugMessageHandler);
    // Allow parameter interaction before we create the options model
    app.parameterSetup();
    GUIUtil::LogQtInfo();
    // Load GUI settings from QSettings
    app.createOptionsModel(false);

    if (did_show_intro) {
        // Store intro dialog settings other than datadir (network specific)
        app.InitializePruneSetting(prune);
    }

    app.createSplashScreen(networkStyle.data());

    int rv = EXIT_SUCCESS;
    try
    {
        app.createWindow(networkStyle.data());
        // Perform base initialization before spinning up initialization/shutdown thread
        // This is acceptable because this function only contains steps that are quick to execute,
        // so the GUI thread won't be held up.
        if (app.baseInitialize()) {
            app.requestInitialize();
#if defined(Q_OS_WIN)
            WinShutdownMonitor::registerShutdownBlockReason(QObject::tr("%1 didn't yet exit safely...").arg(PACKAGE_NAME), (HWND)app.getMainWinId());
#endif
            app.exec();
            app.requestShutdown();
            app.exec();
            rv = app.getReturnValue();
        } else {
            // A dialog with detailed error will have been shown by InitError()
            rv = EXIT_FAILURE;
        }
    } catch (const std::exception& e) {
        //        PrintExceptionContinue(&e, "Runaway exception");
        app.handleRunawayException(QString::fromStdString(node->getWarnings()));
    } catch (...) {
        //        PrintExceptionContinue(nullptr, "Runaway exception");
        app.handleRunawayException(QString::fromStdString(node->getWarnings()));
    }
    return rv;
}
