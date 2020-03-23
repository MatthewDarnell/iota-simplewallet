// Copyright (c) 2011-2019 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <rpcconsole.h>
#include <forms/ui_debugwindow.h>

#include <clientmodel.h>
#include <platformstyle.h>
#include <walletmodel.h>
#include <interfaces/node.h>
#include <util/strencodings.h>
#include <interfaces/wallet.h>
#include <iotaqtconfig.h>

#include <QKeyEvent>
#include <QMenu>
#include <QMessageBox>
#include <QScrollBar>
#include <QScreen>
#include <QSettings>
#include <QTime>
#include <QTimer>
#include <QStringList>
#include <QDebug>
#include <QDir>
#include <QDesktopServices>

// TODO: add a scrollback limit, as there is currently none
// TODO: make it possible to filter out categories (esp debug messages when implemented)
// TODO: receive errors and debug messages through ClientModel

const int CONSOLE_HISTORY = 50;
const int INITIAL_TRAFFIC_GRAPH_MINS = 30;
const QSize FONT_RANGE(4, 40);
const char fontSizeSettingsKey[] = "consoleFontSize";

const struct {
    const char *url;
    const char *source;
} ICON_MAPPING[] = {
{"cmd-request", ":/icons/tx_input"},
{"cmd-reply", ":/icons/tx_output"},
{"cmd-error", ":/icons/tx_output"},
{"misc", ":/icons/tx_inout"},
{nullptr, nullptr}
};

namespace {

// don't add private key handling cmd's to the history
const QStringList historyFilter = QStringList()
        << "importprivkey"
        << "importmulti"
        << "sethdseed"
        << "signmessagewithprivkey"
        << "signrawtransactionwithkey"
        << "walletpassphrase"
        << "walletpassphrasechange"
        << "encryptwallet";

}

/* Object for executing console RPC commands in a separate thread.
*/
class RPCExecutor : public QObject
{
    Q_OBJECT
public:
    explicit RPCExecutor(interfaces::Node& node) : m_node(node) {}

public Q_SLOTS:
    void request(const QString &command, const WalletModel* wallet_model);

Q_SIGNALS:
    void reply(int category, const QString &command);

private:
    interfaces::Node& m_node;
};

/** Class for handling RPC timers
 * (used for e.g. re-locking the wallet after a timeout)
 */
//class QtRPCTimerBase: public QObject/*, public RPCTimerBase*/
//{
//    Q_OBJECT
//public:
//    QtRPCTimerBase(std::function<void()>& _func, int64_t millis):
//        func(_func)
//    {
//        timer.setSingleShot(true);
//        connect(&timer, &QTimer::timeout, [this]{ func(); });
//        timer.start(millis);
//    }
//    ~QtRPCTimerBase() {}
//private:
//    QTimer timer;
//    std::function<void()> func;
//};

//class QtRPCTimerInterface: public RPCTimerInterface
//{
//public:
//    ~QtRPCTimerInterface() {}
//    const char *Name() { return "Qt"; }
//    RPCTimerBase* NewTimer(std::function<void()>& func, int64_t millis)
//    {
//        return new QtRPCTimerBase(func, millis);
//    }
//};


#include <rpcconsole.moc>

/**
 * Split shell command line into a list of arguments and optionally execute the command(s).
 * Aims to emulate \c bash and friends.
 *
 * - Command nesting is possible with parenthesis; for example: validateaddress(getnewaddress())
 * - Arguments are delimited with whitespace or comma
 * - Extra whitespace at the beginning and end and between arguments will be ignored
 * - Text can be "double" or 'single' quoted
 * - The backslash \c \ is used as escape character
 *   - Outside quotes, any character can be escaped
 *   - Within double quotes, only escape \c " and backslashes before a \c " or another backslash
 *   - Within single quotes, no escaping is possible and no special interpretation takes place
 *
 * @param[in]    node    optional node to execute command on
 * @param[out]   strResult   stringified result from the executed command(chain)
 * @param[in]    strCommand  Command line to split
 * @param[in]    fExecute    set true if you want the command to be executed
 * @param[out]   pstrFilteredOut  Command line, filtered to remove any sensitive data
 */

bool RPCConsole::RPCParseCommandLine(interfaces::Node* node, std::string &strResult, const std::string &strCommand, const bool fExecute, std::string * const pstrFilteredOut, const WalletModel* wallet_model)
{
    std::vector< std::vector<std::string> > stack;
    stack.push_back(std::vector<std::string>());

    enum CmdParseState
    {
        STATE_EATING_SPACES,
        STATE_EATING_SPACES_IN_ARG,
        STATE_EATING_SPACES_IN_BRACKETS,
        STATE_ARGUMENT,
        STATE_SINGLEQUOTED,
        STATE_DOUBLEQUOTED,
        STATE_ESCAPE_OUTER,
        STATE_ESCAPE_DOUBLEQUOTED,
        STATE_COMMAND_EXECUTED,
    } state = STATE_EATING_SPACES;
    std::string curarg;
    std::string lastResult;
    unsigned nDepthInsideSensitive = 0;
    size_t filter_begin_pos = 0, chpos;
    std::vector<std::pair<size_t, size_t>> filter_ranges;

    auto add_to_current_stack = [&](const std::string& strArg) {
        if (stack.back().empty() && (!nDepthInsideSensitive) && historyFilter.contains(QString::fromStdString(strArg), Qt::CaseInsensitive)) {
            nDepthInsideSensitive = 1;
            filter_begin_pos = chpos;
        }
        // Make sure stack is not empty before adding something
        if (stack.empty()) {
            stack.push_back(std::vector<std::string>());
        }
        stack.back().push_back(strArg);
    };

    auto close_out_params = [&]() {
        if (nDepthInsideSensitive) {
            if (!--nDepthInsideSensitive) {
                assert(filter_begin_pos);
                filter_ranges.push_back(std::make_pair(filter_begin_pos, chpos));
                filter_begin_pos = 0;
            }
        }
        stack.pop_back();
    };

    std::string strCommandTerminated = strCommand;
    if (strCommandTerminated.back() != '\n')
        strCommandTerminated += "\n";
    for (chpos = 0; chpos < strCommandTerminated.size(); ++chpos)
    {
        char ch = strCommandTerminated[chpos];
        switch(state)
        {
        case STATE_COMMAND_EXECUTED:
        {
            bool breakParsing = true;

            // pop the stack and return the result to the current command arguments
            close_out_params();

            curarg.clear();
            // assume eating space state
            state = STATE_EATING_SPACES;
            if (breakParsing)
                break;
        }
        case STATE_ARGUMENT: // In or after argument
        case STATE_EATING_SPACES_IN_ARG:
        case STATE_EATING_SPACES_IN_BRACKETS:
        case STATE_EATING_SPACES: // Handle runs of whitespace
            switch(ch)
            {
            case '"': state = STATE_DOUBLEQUOTED; break;
            case '\'': state = STATE_SINGLEQUOTED; break;
            case '\\': state = STATE_ESCAPE_OUTER; break;
            case '(': case ')': case '\n':
                if (state == STATE_EATING_SPACES_IN_ARG)
                    throw std::runtime_error("Invalid Syntax");
                if (state == STATE_ARGUMENT)
                {
                    if (ch == '(' && stack.size() && stack.back().size() > 0)
                    {
                        if (nDepthInsideSensitive) {
                            ++nDepthInsideSensitive;
                        }
                        stack.push_back(std::vector<std::string>());
                    }

                    // don't allow commands after executed commands on baselevel
                    if (!stack.size())
                        throw std::runtime_error("Invalid Syntax");

                    add_to_current_stack(curarg);
                    curarg.clear();
                    state = STATE_EATING_SPACES_IN_BRACKETS;
                }
                if ((ch == ')' || ch == '\n') && stack.size() > 0)
                {
                    if (fExecute) {
                        // Convert argument list to JSON objects in method-dependent way,
                        // and pass it along with the method name to the dispatcher.
                        auto params = std::vector<std::string>(stack.back().begin() + 1, stack.back().end());
                        std::string method = stack.back()[0];
                        std::string uri;
                        if (wallet_model) {
                            QByteArray encodedName = QUrl::toPercentEncoding(wallet_model->getWalletName());
                            uri = "/wallet/"+std::string(encodedName.constData(), encodedName.length());
                        }
                        assert(node);
                        lastResult = node->executeRpc(method, params, uri);
                    }

                    state = STATE_COMMAND_EXECUTED;
                    curarg.clear();
                }
                break;
            case ' ': case ',': case '\t':
                if(state == STATE_EATING_SPACES_IN_ARG && curarg.empty() && ch == ',')
                    throw std::runtime_error("Invalid Syntax");

                else if(state == STATE_ARGUMENT) // Space ends argument
                {
                    add_to_current_stack(curarg);
                    curarg.clear();
                }
                if ((state == STATE_EATING_SPACES_IN_BRACKETS || state == STATE_ARGUMENT) && ch == ',')
                {
                    state = STATE_EATING_SPACES_IN_ARG;
                    break;
                }
                state = STATE_EATING_SPACES;
                break;
            default: curarg += ch; state = STATE_ARGUMENT;
            }
            break;
        case STATE_SINGLEQUOTED: // Single-quoted string
            switch(ch)
            {
            case '\'': state = STATE_ARGUMENT; break;
            default: curarg += ch;
            }
            break;
        case STATE_DOUBLEQUOTED: // Double-quoted string
            switch(ch)
            {
            case '"': state = STATE_ARGUMENT; break;
            case '\\': state = STATE_ESCAPE_DOUBLEQUOTED; break;
            default: curarg += ch;
            }
            break;
        case STATE_ESCAPE_OUTER: // '\' outside quotes
            curarg += ch; state = STATE_ARGUMENT;
            break;
        case STATE_ESCAPE_DOUBLEQUOTED: // '\' in double-quoted text
            if(ch != '"' && ch != '\\') curarg += '\\'; // keep '\' for everything but the quote and '\' itself
            curarg += ch; state = STATE_DOUBLEQUOTED;
            break;
        }
    }
    if (pstrFilteredOut) {
        if (STATE_COMMAND_EXECUTED == state) {
            assert(!stack.empty());
            close_out_params();
        }
        *pstrFilteredOut = strCommand;
        for (auto i = filter_ranges.rbegin(); i != filter_ranges.rend(); ++i) {
            pstrFilteredOut->replace(i->first, i->second - i->first, "(…)");
        }
    }

    switch(state) // final state
    {
    case STATE_COMMAND_EXECUTED:
        strResult = lastResult;
    case STATE_ARGUMENT:
    case STATE_EATING_SPACES:
        return true;
    default: // ERROR to end in one of the other states
        return false;
    }
}

void RPCExecutor::request(const QString &command, const WalletModel* wallet_model)
{
    try
    {
        std::string result;
        std::string executableCommand = command.toStdString() + "\n";

        // Catch the console-only-help command before RPC call is executed and reply with help text as-if a RPC reply.
        if(executableCommand == "help-console\n") {
            Q_EMIT reply(RPCConsole::CMD_REPLY, QString(("\n"
                                                         "This console accepts RPC commands using the standard syntax.\n"
                                                         "   example:    getblockhash 0\n\n"

                                                         "This console can also accept RPC commands using the parenthesized syntax.\n"
                                                         "   example:    getblockhash(0)\n\n"

                                                         "Commands may be nested when specified with the parenthesized syntax.\n"
                                                         "   example:    getblock(getblockhash(0) 1)\n\n"

                                                         "A space or a comma can be used to delimit arguments for either syntax.\n"
                                                         "   example:    getblockhash 0\n"
                                                         "               getblockhash,0\n\n"

                                                         "Named results can be queried with a non-quoted key string in brackets using the parenthesized syntax.\n"
                                                         "   example:    getblock(getblockhash(0) 1)[tx]\n\n"

                                                         "Results without keys can be queried with an integer in brackets using the parenthesized syntax.\n"
                                                         "   example:    getblock(getblockhash(0),1)[tx][0]\n\n")));
            return;
        }
        if (!RPCConsole::RPCExecuteCommandLine(m_node, result, executableCommand, nullptr, wallet_model)) {
            Q_EMIT reply(RPCConsole::CMD_ERROR, QString("Parse error: unbalanced ' or \""));
            return;
        }

        Q_EMIT reply(RPCConsole::CMD_REPLY, QString::fromStdString(result));
    }
    //    catch (UniValue& objError)
    //    {
    //        try // Nice formatting for standard-format error
    //        {
    //            int code = find_value(objError, "code").get_int();
    //            std::string message = find_value(objError, "message").get_str();
    //            Q_EMIT reply(RPCConsole::CMD_ERROR, QString::fromStdString(message) + " (code " + QString::number(code) + ")");
    //        }
    //        catch (const std::runtime_error&) // raised when converting to invalid type, i.e. missing code or message
    //        {   // Show raw JSON object
    //            Q_EMIT reply(RPCConsole::CMD_ERROR, QString::fromStdString(objError.write()));
    //        }
    //    }
    catch (const std::exception& e)
    {
        Q_EMIT reply(RPCConsole::CMD_ERROR, QString("Error: ") + QString::fromStdString(e.what()));
    }
}

RPCConsole::RPCConsole(interfaces::Node& node, const PlatformStyle *_platformStyle, QWidget *parent) :
    QWidget(parent),
    m_node(node),
    ui(new Ui::RPCConsole),
    platformStyle(_platformStyle)
{
    ui->setupUi(this);
    QSettings settings;
    if (!restoreGeometry(settings.value("RPCConsoleWindowGeometry").toByteArray())) {
        // Restore failed (perhaps missing setting), center the window
        move(QGuiApplication::primaryScreen()->availableGeometry().center() - frameGeometry().center());
    }

    QChar nonbreaking_hyphen(8209);
    ui->dataDir->setToolTip(ui->dataDir->toolTip().arg(QString(nonbreaking_hyphen) + "datadir"));
    ui->openDebugLogfileButton->setToolTip(ui->openDebugLogfileButton->toolTip().arg(PACKAGE_NAME));

    if (platformStyle->getImagesOnButtons()) {
        ui->openDebugLogfileButton->setIcon(platformStyle->SingleColorIcon(":/icons/export"));
    }
    ui->clearButton->setIcon(platformStyle->SingleColorIcon(":/icons/remove"));
    ui->fontBiggerButton->setIcon(platformStyle->SingleColorIcon(":/icons/fontbigger"));
    ui->fontSmallerButton->setIcon(platformStyle->SingleColorIcon(":/icons/fontsmaller"));

    // Install event filter for up and down arrow
    ui->lineEdit->installEventFilter(this);
    ui->messagesWidget->installEventFilter(this);

    connect(ui->clearButton, &QPushButton::clicked, this, &RPCConsole::clear);
    connect(ui->fontBiggerButton, &QPushButton::clicked, this, &RPCConsole::fontBigger);
    connect(ui->fontSmallerButton, &QPushButton::clicked, this, &RPCConsole::fontSmaller);

    // disable the wallet selector by default
    ui->WalletSelector->setVisible(false);
    ui->WalletSelectorLabel->setVisible(false);

    // set library version labels
    //    ui->berkeleyDBVersion->setText(DbEnv::version(nullptr, nullptr, nullptr));
    // Register RPC timer interface
    //    rpcTimerInterface = new QtRPCTimerInterface();
    // avoid accidentally overwriting an existing, non QTThread
    // based timer interface
    m_node.rpcSetTimerInterfaceIfUnset(rpcTimerInterface);

    consoleFontSize = settings.value(fontSizeSettingsKey, QFontInfo(QFont()).pointSize()).toInt();
    clear();
}

RPCConsole::~RPCConsole()
{
    QSettings settings;
    settings.setValue("RPCConsoleWindowGeometry", saveGeometry());
    m_node.rpcUnsetTimerInterface(rpcTimerInterface);
    //    delete rpcTimerInterface;
    delete ui;
}

bool RPCConsole::eventFilter(QObject* obj, QEvent *event)
{
    if(event->type() == QEvent::KeyPress) // Special key handling
    {
        QKeyEvent *keyevt = static_cast<QKeyEvent*>(event);
        int key = keyevt->key();
        Qt::KeyboardModifiers mod = keyevt->modifiers();
        switch(key)
        {
        case Qt::Key_Up: if(obj == ui->lineEdit) { browseHistory(-1); return true; } break;
        case Qt::Key_Down: if(obj == ui->lineEdit) { browseHistory(1); return true; } break;
        case Qt::Key_PageUp: /* pass paging keys to messages widget */
        case Qt::Key_PageDown:
            if(obj == ui->lineEdit)
            {
                QApplication::postEvent(ui->messagesWidget, new QKeyEvent(*keyevt));
                return true;
            }
            break;
        case Qt::Key_Return:
        case Qt::Key_Enter:
            // forward these events to lineEdit
            if(obj == autoCompleter->popup()) {
                QApplication::postEvent(ui->lineEdit, new QKeyEvent(*keyevt));
                autoCompleter->popup()->hide();
                return true;
            }
            break;
        default:
            // Typing in messages widget brings focus to line edit, and redirects key there
            // Exclude most combinations and keys that emit no text, except paste shortcuts
            if(obj == ui->messagesWidget && (
                        (!mod && !keyevt->text().isEmpty() && key != Qt::Key_Tab) ||
                        ((mod & Qt::ControlModifier) && key == Qt::Key_V) ||
                        ((mod & Qt::ShiftModifier) && key == Qt::Key_Insert)))
            {
                ui->lineEdit->setFocus();
                QApplication::postEvent(ui->lineEdit, new QKeyEvent(*keyevt));
                return true;
            }
        }
    }
    return QWidget::eventFilter(obj, event);
}

void RPCConsole::setClientModel(ClientModel *model)
{
    clientModel = model;

    if (model) {
        // Keep up to date with client
        //        setNumConnections(model->getNumConnections());
        connect(model, &ClientModel::numConnectionsChanged, this, &RPCConsole::setNumConnections);

        interfaces::Node& node = clientModel->node();
        setAppInfo(node.getAppName(), node.getAppVersion());
        setNumBlocks(node.getNumBlocks(), node.getLatestMilestone());
        connect(model, &ClientModel::appInfoChanged, this, &RPCConsole::setAppInfo);
        connect(model, &ClientModel::numBlocksChanged, this, &RPCConsole::setNumBlocks);
        connect(model, &ClientModel::updateNumConnections, this, &RPCConsole::setNumConnections);

        updateNetworkState();
        connect(model, &ClientModel::networkActiveChanged, this, &RPCConsole::setNetworkActive);

        // Provide initial values
        ui->clientVersion->setText(model->formatFullVersion());
        ui->dataDir->setText(model->dataDir());
        ui->startupTime->setText(model->formatClientStartupTime());

        //Setup autocomplete and attach it
        QStringList wordList;
        std::vector<std::string> commandList = m_node.listRpcCommands();
        for (size_t i = 0; i < commandList.size(); ++i)
        {
            wordList << commandList[i].c_str();
            wordList << ("help " + commandList[i]).c_str();
        }

        wordList << "help-console";
        wordList.sort();
        autoCompleter = new QCompleter(wordList, this);
        autoCompleter->setModelSorting(QCompleter::CaseSensitivelySortedModel);
        // ui->lineEdit is initially disabled because running commands is only
        // possible from now on.
        ui->lineEdit->setEnabled(true);
        ui->lineEdit->setCompleter(autoCompleter);
        autoCompleter->popup()->installEventFilter(this);
        // Start thread to execute RPC commands.
        startExecutor();
    }
    if (!model) {
        // Client model is being set to 0, this means shutdown() is about to be called.
        thread.quit();
        thread.wait();
    }
}

void RPCConsole::addWallet(WalletModel * const walletModel)
{
    // use name for text and wallet model for internal data object (to allow to move to a wallet id later)
    ui->WalletSelector->addItem(walletModel->getDisplayName(), QVariant::fromValue(walletModel));
    if (ui->WalletSelector->count() == 2 && !isVisible()) {
        // First wallet added, set to default so long as the window isn't presently visible (and potentially in use)
        ui->WalletSelector->setCurrentIndex(1);
    }
    if (ui->WalletSelector->count() > 2) {
        ui->WalletSelector->setVisible(true);
        ui->WalletSelectorLabel->setVisible(true);
    }
}

void RPCConsole::removeWallet(WalletModel * const walletModel)
{
    ui->WalletSelector->removeItem(ui->WalletSelector->findData(QVariant::fromValue(walletModel)));
    if (ui->WalletSelector->count() == 2) {
        ui->WalletSelector->setVisible(false);
        ui->WalletSelectorLabel->setVisible(false);
    }
}

static QString categoryClass(int category)
{
    switch(category)
    {
    case RPCConsole::CMD_REQUEST:  return "cmd-request"; break;
    case RPCConsole::CMD_REPLY:    return "cmd-reply"; break;
    case RPCConsole::CMD_ERROR:    return "cmd-error"; break;
    default:                       return "misc";
    }
}

void RPCConsole::fontBigger()
{
    setFontSize(consoleFontSize+1);
}

void RPCConsole::fontSmaller()
{
    setFontSize(consoleFontSize-1);
}

void RPCConsole::setFontSize(int newSize)
{
    QSettings settings;

    //don't allow an insane font size
    if (newSize < FONT_RANGE.width() || newSize > FONT_RANGE.height())
        return;

    // temp. store the console content
    QString str = ui->messagesWidget->toHtml();

    // replace font tags size in current content
    str.replace(QString("font-size:%1pt").arg(consoleFontSize), QString("font-size:%1pt").arg(newSize));

    // store the new font size
    consoleFontSize = newSize;
    settings.setValue(fontSizeSettingsKey, consoleFontSize);

    // clear console (reset icon sizes, default stylesheet) and re-add the content
    float oldPosFactor = 1.0 / ui->messagesWidget->verticalScrollBar()->maximum() * ui->messagesWidget->verticalScrollBar()->value();
    clear(false);
    ui->messagesWidget->setHtml(str);
    ui->messagesWidget->verticalScrollBar()->setValue(oldPosFactor * ui->messagesWidget->verticalScrollBar()->maximum());
}

void RPCConsole::clear(bool clearHistory)
{
    ui->messagesWidget->clear();
    if(clearHistory)
    {
        history.clear();
        historyPtr = 0;
    }
    ui->lineEdit->clear();
    ui->lineEdit->setFocus();

    // Add smoothly scaled icon images.
    // (when using width/height on an img, Qt uses nearest instead of linear interpolation)
    for(int i=0; ICON_MAPPING[i].url; ++i)
    {
        ui->messagesWidget->document()->addResource(
                    QTextDocument::ImageResource,
                    QUrl(ICON_MAPPING[i].url),
                    platformStyle->SingleColorImage(ICON_MAPPING[i].source).scaled(QSize(consoleFontSize*2, consoleFontSize*2), Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
    }

    // Set default style sheet
    QFontInfo fixedFontInfo(GUIUtil::fixedPitchFont());
    ui->messagesWidget->document()->setDefaultStyleSheet(
                QString(
                    "table { }"
                    "td.time { color: #808080; font-size: %2; padding-top: 3px; } "
                    "td.message { font-family: %1; font-size: %2; white-space:pre-wrap; } "
                    "td.cmd-request { color: #006060; } "
                    "td.cmd-error { color: red; } "
                    ".secwarning { color: red; }"
                    "b { color: #006060; } "
                    ).arg(fixedFontInfo.family(), QString("%1pt").arg(consoleFontSize))
                );

#ifdef Q_OS_MAC
    QString clsKey = "(⌘)-L";
#else
    QString clsKey = "Ctrl-L";
#endif

    message(CMD_REPLY, (tr("Welcome to the %1 RPC console.").arg(PACKAGE_NAME) + "<br>" +
                        tr("Use up and down arrows to navigate history, and %1 to clear screen.").arg("<b>"+clsKey+"</b>") + "<br>" +
                        tr("Type %1 for an overview of available commands.").arg("<b>help</b>") + "<br>" +
                        tr("For more information on using this console type %1.").arg("<b>help-console</b>") +
                        "<br><span class=\"secwarning\"><br>" +
                        tr("WARNING: Scammers have been active, telling users to type commands here, stealing their wallet contents. Do not use this console without fully understanding the ramifications of a command.") +
                        "</span>"),
            true);
}

void RPCConsole::keyPressEvent(QKeyEvent *event)
{
    if(windowType() != Qt::Widget && event->key() == Qt::Key_Escape)
    {
        close();
    }
}

void RPCConsole::message(int category, const QString &message, bool html)
{
    QTime time = QTime::currentTime();
    QString timeString = time.toString();
    QString out;
    out += "<table><tr><td class=\"time\" width=\"65\">" + timeString + "</td>";
    out += "<td class=\"icon\" width=\"32\"><img src=\"" + categoryClass(category) + "\"></td>";
    out += "<td class=\"message " + categoryClass(category) + "\" valign=\"middle\">";
    if(html)
        out += message;
    else
        out += GUIUtil::HtmlEscape(message, false);
    out += "</td></tr></table>";
    ui->messagesWidget->append(out);
}

void RPCConsole::updateNetworkState()
{   
    if(!clientModel->node().getNetworkActive()) {
        QString  connections = " (" + tr("Network activity disabled") + ")";
        ui->numberOfConnections->setText(connections);
    }
}

void RPCConsole::setNumConnections(int count)
{
    if (!clientModel)
        return;

    ui->numberOfConnections->setText(QString::number(count));

    updateNetworkState();
}

void RPCConsole::setNetworkActive(bool networkActive)
{
    updateNetworkState();
}

void RPCConsole::setNumBlocks(int count, QString blockTip)
{
    ui->latestMilestone->setText(blockTip);
}

void RPCConsole::setAppInfo(QString appName, QString appVersion)
{
    ui->networkName->setText(appName);
    ui->clientVersion->setText(appVersion);
}

void RPCConsole::on_lineEdit_returnPressed()
{
    QString cmd = ui->lineEdit->text();

    if(!cmd.isEmpty())
    {
        std::string strFilteredCmd;
        try {
            std::string dummy;
            if (!RPCParseCommandLine(nullptr, dummy, cmd.toStdString(), false, &strFilteredCmd)) {
                // Failed to parse command, so we cannot even filter it for the history
                throw std::runtime_error("Invalid command line");
            }
        } catch (const std::exception& e) {
            QMessageBox::critical(this, "Error", QString("Error: ") + QString::fromStdString(e.what()));
            return;
        }

        ui->lineEdit->clear();

        cmdBeforeBrowsing = QString();

        WalletModel* wallet_model = ui->WalletSelector->currentData().value<WalletModel*>();

        if (m_last_wallet_model != wallet_model) {
            if (wallet_model) {
                message(CMD_REQUEST, tr("Executing command using \"%1\" wallet").arg(wallet_model->getWalletName()));
            } else {
                message(CMD_REQUEST, tr("Executing command without any wallet"));
            }
            m_last_wallet_model = wallet_model;
        }

        message(CMD_REQUEST, QString::fromStdString(strFilteredCmd));
        Q_EMIT cmdRequest(cmd, m_last_wallet_model);

        cmd = QString::fromStdString(strFilteredCmd);

        // Remove command, if already in history
        history.removeOne(cmd);
        // Append command to history
        history.append(cmd);
        // Enforce maximum history size
        while(history.size() > CONSOLE_HISTORY)
            history.removeFirst();
        // Set pointer to end of history
        historyPtr = history.size();

        // Scroll console view to end
        scrollToEnd();
    }
}

void RPCConsole::browseHistory(int offset)
{
    // store current text when start browsing through the history
    if (historyPtr == history.size()) {
        cmdBeforeBrowsing = ui->lineEdit->text();
    }

    historyPtr += offset;
    if(historyPtr < 0)
        historyPtr = 0;
    if(historyPtr > history.size())
        historyPtr = history.size();
    QString cmd;
    if(historyPtr < history.size())
        cmd = history.at(historyPtr);
    else if (!cmdBeforeBrowsing.isNull()) {
        cmd = cmdBeforeBrowsing;
    }
    ui->lineEdit->setText(cmd);
}

void RPCConsole::startExecutor()
{
    RPCExecutor *executor = new RPCExecutor(m_node);
    executor->moveToThread(&thread);

    // Replies from executor object must go to this object
    connect(executor, &RPCExecutor::reply, this, static_cast<void (RPCConsole::*)(int, const QString&)>(&RPCConsole::message));

    // Requests from this object must go to executor
    connect(this, &RPCConsole::cmdRequest, executor, &RPCExecutor::request);

    // Make sure executor object is deleted in its own thread
    connect(&thread, &QThread::finished, executor, &RPCExecutor::deleteLater);

    // Default implementation of QThread::run() simply spins up an event loop in the thread,
    // which is what we want.
    thread.start();
}

void RPCConsole::on_tabWidget_currentChanged(int index)
{
    if (ui->tabWidget->widget(index) == ui->tab_console) {
        ui->lineEdit->setFocus();
    }
}

void RPCConsole::on_openDebugLogfileButton_clicked()
{
    if (!clientModel) {
        return;
    }

    auto pathDebug = QDir(clientModel->dataDir()).absoluteFilePath("iota-simplewallet.log");
    if (QFileInfo(pathDebug).exists())
        QDesktopServices::openUrl(QUrl::fromLocalFile(pathDebug));

}

void RPCConsole::scrollToEnd()
{
    QScrollBar *scrollbar = ui->messagesWidget->verticalScrollBar();
    scrollbar->setValue(scrollbar->maximum());
}

void RPCConsole::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
}

void RPCConsole::showEvent(QShowEvent *event)
{
    QWidget::showEvent(event);

    if (!clientModel)
        return;
}

void RPCConsole::hideEvent(QHideEvent *event)
{
    QWidget::hideEvent(event);

    if (!clientModel)
        return;
}

void RPCConsole::updateClientVersion(const QString &version)
{
    ui->clientVersion->setText(version);
}

void RPCConsole::setTabFocus(enum TabTypes tabType)
{
    ui->tabWidget->setCurrentIndex(int(tabType));
}

QString RPCConsole::tabTitle(TabTypes tab_type) const
{
    return ui->tabWidget->tabText(int(tab_type));
}

QKeySequence RPCConsole::tabShortcut(TabTypes tab_type) const
{
    switch (tab_type) {
    case TabTypes::INFO: return QKeySequence(Qt::CTRL + Qt::Key_I);
    case TabTypes::CONSOLE: return QKeySequence(Qt::CTRL + Qt::Key_T);
    } // no default case, so the compiler can warn about missing cases

    assert(false);
}

void RPCConsole::updateAlerts(const QString& warnings)
{
    this->ui->label_alerts->setVisible(!warnings.isEmpty());
    this->ui->label_alerts->setText(warnings);
}

//#endif
