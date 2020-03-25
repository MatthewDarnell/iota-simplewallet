// Copyright (c) 2011-2019 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <clientmodel.h>

#include <guiconstants.h>
#include <guiutil.h>

//#include <clientversion.h>
#include <interfaces/handler.h>
#include <interfaces/node.h>

#include <stdint.h>

#include <QDebug>
#include <QThread>
#include <QTimer>

static int64_t nLastHeaderTipUpdateNotification = 0;
static int64_t nLastBlockTipUpdateNotification = 0;

ClientModel::ClientModel(interfaces::Node& node, OptionsModel *_optionsModel, QObject *parent) :
    QObject(parent),
    m_node(node),
    optionsModel(_optionsModel),
    m_thread(new QThread(this))
{
    cachedBestHeaderHeight = -1;
    cachedBestHeaderTime = -1;

    subscribeToCoreSignals();
}

ClientModel::~ClientModel()
{
    unsubscribeFromCoreSignals();

    m_thread->quit();
    m_thread->wait();
}

int ClientModel::getNumConnections(unsigned int flags) const
{
    //    CConnman::NumConnections connections = CConnman::CONNECTIONS_NONE;

    //    if(flags == CONNECTIONS_IN)
    //        connections = CConnman::CONNECTIONS_IN;
    //    else if (flags == CONNECTIONS_OUT)
    //        connections = CConnman::CONNECTIONS_OUT;
    //    else if (flags == CONNECTIONS_ALL)
    //        connections = CConnman::CONNECTIONS_ALL;

    //    return m_node.getNodeCount(connections);
    return 34;
}

int ClientModel::getHeaderTipHeight() const
{
    if (cachedBestHeaderHeight == -1) {
        // make sure we initially populate the cache via a cs_main lock
        // otherwise we need to wait for a tip update
        int height;
        int64_t blockTime;
        if (m_node.getHeaderTip(height, blockTime)) {
            cachedBestHeaderHeight = height;
            cachedBestHeaderTime = blockTime;
        }
    }
    return cachedBestHeaderHeight;
}

int64_t ClientModel::getHeaderTipTime() const
{
    if (cachedBestHeaderTime == -1) {
        int height;
        int64_t blockTime;
        if (m_node.getHeaderTip(height, blockTime)) {
            cachedBestHeaderHeight = height;
            cachedBestHeaderTime = blockTime;
        }
    }
    return cachedBestHeaderTime;
}

void ClientModel::updateNumConnections(int numConnections)
{
    Q_EMIT numConnectionsChanged(numConnections);
}

void ClientModel::updateNetworkActive(bool networkActive)
{
    Q_EMIT networkActiveChanged(networkActive);
}

void ClientModel::updateAlert()
{
    Q_EMIT alertsChanged(getStatusBarWarnings());
}

enum BlockSource ClientModel::getBlockSource() const
{
    if (m_node.getReindex())
        return BlockSource::REINDEX;
    else if (m_node.getImporting())
        return BlockSource::DISK;
    else if (getNumConnections() > 0)
        return BlockSource::NETWORK;

    return BlockSource::NONE;
}

QString ClientModel::getStatusBarWarnings() const
{
    return QString::fromStdString(m_node.getWarnings());
}

OptionsModel *ClientModel::getOptionsModel()
{
    return optionsModel;
}

QString ClientModel::formatFullVersion() const
{
    return QString{"N/A"};
    //    return QString::fromStdString(FormatFullVersion());
}

QString ClientModel::formatSubVersion() const
{
    return QString{"N/A"};
    //    return QString::fromStdString(strSubVersion);
}

bool ClientModel::isReleaseVersion() const
{
    return true;
    //    return CLIENT_VERSION_IS_RELEASE;
}

QString ClientModel::formatClientStartupTime() const
{
    return {};
    //    return QDateTime::fromTime_t(GetStartupTime()).toString();
}

QString ClientModel::dataDir() const
{
    return QString::fromStdString(m_node.getDataDir());
}

// Handlers for core signals
static void ShowProgress(ClientModel *clientmodel, const std::string &title, int nProgress)
{
    // emits signal "showProgress"
    bool invoked = QMetaObject::invokeMethod(clientmodel, "showProgress", Qt::QueuedConnection,
                                             Q_ARG(QString, QString::fromStdString(title)),
                                             Q_ARG(int, nProgress));
    assert(invoked);
}

static void NotifyNumConnectionsChanged(ClientModel *clientmodel, int newNumConnections)
{
    // Too noisy: qDebug() << "NotifyNumConnectionsChanged: " + QString::number(newNumConnections);
    bool invoked = QMetaObject::invokeMethod(clientmodel, "updateNumConnections", Qt::QueuedConnection,
                                             Q_ARG(int, newNumConnections));
    assert(invoked);
}

static void NotifyNetworkActiveChanged(ClientModel *clientmodel, bool networkActive)
{
    bool invoked = QMetaObject::invokeMethod(clientmodel, "updateNetworkActive", Qt::QueuedConnection,
                                             Q_ARG(bool, networkActive));
    assert(invoked);
}

static void NotifyAlertChanged(ClientModel *clientmodel)
{
    qDebug() << "NotifyAlertChanged";
    bool invoked = QMetaObject::invokeMethod(clientmodel, "updateAlert", Qt::QueuedConnection);
    assert(invoked);
}

static void BannedListChanged(ClientModel *clientmodel)
{
    qDebug() << QString("%1: Requesting update for peer banlist").arg(__func__);
    bool invoked = QMetaObject::invokeMethod(clientmodel, "updateBanlist", Qt::QueuedConnection);
    assert(invoked);
}

static void BlockTipChanged(ClientModel *clientmodel, int height, std::string blockTip)
{
    // lock free async UI updates in case we have a new block tip
    // during initial sync, only update the UI if the last update
    // was > 250ms (MODEL_UPDATE_DELAY) ago
    int64_t now = 0;
    //    if (initialSync)
    //        now = GetTimeMillis();

    int64_t& nLastUpdateNotification = nLastBlockTipUpdateNotification;

    // During initial sync, block notifications, and header notifications from reindexing are both throttled.
    //    if (!initialSync || (fHeader && !clientmodel->node().getReindex()) || now - nLastUpdateNotification > MODEL_UPDATE_DELAY) {
    //pass an async signal to the UI thread
    bool invoked = QMetaObject::invokeMethod(clientmodel, "numBlocksChanged", Qt::QueuedConnection,
                                             Q_ARG(int, height),
                                             Q_ARG(QString, QString::fromStdString(blockTip)));
    assert(invoked);
    nLastUpdateNotification = now;
    //    }
}

static void AppInfoChanged(ClientModel *clientModel, std::string appName, std::string appVersion, std::string connectedNode)
{
    bool invoked = QMetaObject::invokeMethod(clientModel, "appInfoChanged", Qt::QueuedConnection,
                                             Q_ARG(QString, QString::fromStdString(appName)),
                                             Q_ARG(QString, QString::fromStdString(appVersion)),
                                             Q_ARG(QString, QString::fromStdString(connectedNode)));

    assert(invoked);
}

void ClientModel::subscribeToCoreSignals()
{
    // Connect signals to client
    m_handler_show_progress = m_node.handleShowProgress(std::bind(ShowProgress, this, std::placeholders::_1, std::placeholders::_2));
    m_handler_notify_num_connections_changed = m_node.handleNotifyNumConnectionsChanged(std::bind(NotifyNumConnectionsChanged, this, std::placeholders::_1));
    m_handler_notify_network_active_changed = m_node.handleNotifyNetworkActiveChanged(std::bind(NotifyNetworkActiveChanged, this, std::placeholders::_1));
    m_handler_notify_alert_changed = m_node.handleNotifyAlertChanged(std::bind(NotifyAlertChanged, this));
    m_handler_banned_list_changed = m_node.handleBannedListChanged(std::bind(BannedListChanged, this));
    m_handler_notify_block_tip = m_node.handleNotifyBlockTip(std::bind(BlockTipChanged, this, std::placeholders::_1, std::placeholders::_2));
    m_handler_notify_appInfo_changed = m_node.handleNotifyAppInfochanged(std::bind(AppInfoChanged, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
}

void ClientModel::unsubscribeFromCoreSignals()
{
    // Disconnect signals from client
    m_handler_show_progress->disconnect();
    m_handler_notify_num_connections_changed->disconnect();
    m_handler_notify_network_active_changed->disconnect();
    m_handler_notify_alert_changed->disconnect();
    m_handler_banned_list_changed->disconnect();
    m_handler_notify_block_tip->disconnect();
    m_handler_notify_appInfo_changed->disconnect();
}

bool ClientModel::getProxyInfo(std::string& ip_port) const
{
    //    proxyType ipv4, ipv6;
    //    if (m_node.getProxy((Network) 1, ipv4) && m_node.getProxy((Network) 2, ipv6)) {
    //      ip_port = ipv4.proxy.ToStringIPPort();
    //      return true;
    //    }
    return false;
}
