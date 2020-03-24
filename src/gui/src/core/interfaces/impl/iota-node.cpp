// Copyright (c) 2018-2019 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "iota-node.hpp"

#include <QJsonDocument>
#include <QJsonValue>
#include <QJsonArray>
#include <QPointer>
#include <QDebug>
#include <QCoreApplication>
#include <QStandardPaths>
#include <QDir>
#include <sstream>

#include <iota/iota-simplewallet.h>

namespace interfaces {

static std::vector<QPointer<IotaNode>> _nodes;


static void ForEachDeliverEvent(std::function<void(IotaNode&)> functor)
{
    _nodes.erase(std::remove_if(std::begin(_nodes), std::end(_nodes), [](const auto &node) {
        return node.isNull();
    }), std::end(_nodes));

    std::for_each(std::begin(_nodes), std::end(_nodes), [functor](auto &node) {
        functor(*node);
    });
}

void* IotaNode::OnBalanceChanged(const char *payload)
{
    auto obj = QJsonDocument::fromJson(QByteArray(payload)).object();
    ForEachDeliverEvent([username = obj.value("username").toString(),
                        balance = obj.value("balance").toString()](IotaNode &node) {
        Q_EMIT node.balanceChanged(username, balance);
    });
    return nullptr;
}

void* IotaNode::OnNodeUpdated(const char *payload)
{
    auto obj = QJsonDocument::fromJson(QByteArray(payload)).object();
    ForEachDeliverEvent([obj](IotaNode &node) {
        auto ptr = &node;
        QMetaObject::invokeMethod(ptr, [=] {
            ptr->updateNodeStatus(obj);
        });
    });
    return nullptr;
}

void* IotaNode::OnTransactionReceived(const char *payload)
{
    auto obj = QJsonDocument::fromJson(QByteArray(payload)).object();
    ForEachDeliverEvent([obj, username = obj.value("username").toString()](IotaNode &node) {
        Q_EMIT node.transactionChanged(username, obj);
    });
    return nullptr;
}

void* IotaNode::OnTransactionSent(const char *payload)
{
    auto obj = QJsonDocument::fromJson(QByteArray(payload)).object();
    ForEachDeliverEvent([obj, username = obj.value("username").toString()](IotaNode &node) {
        Q_EMIT node.transactionChanged(username, obj);
    });
    return nullptr;
}

void* IotaNode::OnSentTransactionConfirmed(const char *payload)
{
    auto obj = QJsonDocument::fromJson(QByteArray(payload)).object();
    ForEachDeliverEvent([obj, username = obj.value("username").toString()](IotaNode &node) {
        Q_EMIT node.transactionChanged(username, obj);
    });
    return nullptr;
}

void* IotaNode::OnReceivedTransactionConfirmed(const char *payload)
{
    auto obj = QJsonDocument::fromJson(QByteArray(payload)).object();
    ForEachDeliverEvent([obj, username = obj.value("username").toString()](IotaNode &node) {
        Q_EMIT node.transactionChanged(username, obj);
    });
    return nullptr;
}

void IotaNode::initError(const std::string &message)
{
}

bool IotaNode::parseParameters(int argc, const char * const argv[], std::string &error)
{
    return true;
}

bool IotaNode::readConfigFiles(std::string &error)
{
    return true;
}

void IotaNode::selectParams(const std::string &network)
{
}

uint64_t IotaNode::getAssumedBlockchainSize()
{
    return 0;
}

uint64_t IotaNode::getAssumedChainStateSize()
{
    return 0;
}

std::string IotaNode::getNetwork()
{
    return {};
}

std::string IotaNode::getDataDir()
{
    static const auto datadir = QStandardPaths::writableLocation(QStandardPaths::DataLocation).toStdString();
    return datadir;
}

void IotaNode::initLogging()
{
    //    init_logger();
}

void IotaNode::initParameterInteraction()
{
}

std::string IotaNode::getWarnings()
{
    return {};
}

uint32_t IotaNode::getLogCategories()
{
    return 0;
}

bool IotaNode::baseInitialize()
{
    auto dataDirPath = getDataDir();

    QDir dataDir(QString::fromStdString(dataDirPath));
    if (!dataDir.exists()) {
        if (!dataDir.mkpath(".")) {
            qDebug("Failed to create dataDir at location: %s\n", dataDirPath.c_str());
            return false;
        }
    }

    init_iota_simplewallet(dataDirPath.data());
    return true;
}

bool IotaNode::appInitMain()
{
    init_db();
    init_crypto();
    init_iota();
    init_events();
    start_threads();

    registerEvents();
    loadAccounts();

    return true;
}

void IotaNode::appShutdown()
{
    shutdown_threads();
    join_threads();
    shutdown_iota();
    shutdown_iota_simplewallet();
    shutdown_config();
    shutdown_events();
}

void IotaNode::startShutdown()
{
}

bool IotaNode::shutdownRequested()
{
    return false;
}

void IotaNode::setupServerArgs()
{
}

bool IotaNode::getHeaderTip(int &height, int64_t &block_time)
{
    return false;
}

int IotaNode::getNumBlocks()
{
    return _latestMilestone.first;
}

QString IotaNode::getLatestMilestone()
{
    return _latestMilestone.second;
}

std::string IotaNode::executeRpc(const std::string &command, const std::vector<std::string> params, const std::string &uri)
{
    std::stringstream ss;
    ss << command;
    for(auto &&param : params)
    {
        ss << " " << param;
    }
    c_string_unique_ptr result(parse_debug_command(ss.str().data()));
    return std::string(result.get());
}

QString IotaNode::getAppName()
{
    return _appInfo.first;
}

QString IotaNode::getAppVersion()
{
    return _appInfo.second;
}

int64_t IotaNode::getLastBlockTime()
{
    return 0;
}

double IotaNode::getVerificationProgress()
{
    return 0;
}

bool IotaNode::isInitialBlockDownload()
{
    return false;
}

bool IotaNode::getReindex()
{
    return false;
}

bool IotaNode::getImporting()
{
    return false;
}

void IotaNode::setNetworkActive(bool active)
{
}

bool IotaNode::getNetworkActive()
{
    return true;
}

std::vector<std::string> IotaNode::listRpcCommands()
{
    return {};
}

void IotaNode::rpcSetTimerInterfaceIfUnset(RPCTimerInterface *iface)
{
}

void IotaNode::rpcUnsetTimerInterface(RPCTimerInterface *iface)
{
}

std::string IotaNode::getWalletDir()
{
    return {};
}

std::vector<std::string> IotaNode::listWalletDir()
{
    return {};
}

std::vector<std::unique_ptr<Wallet> > IotaNode::getWallets()
{
    std::vector<std::unique_ptr<Wallet>> res;
    for(auto &&acc : _accounts)
    {
        res.emplace_back(createIotaWallet(acc));
    }

    return res;
}

std::unique_ptr<Wallet> IotaNode::loadWallet(const std::string &name, std::string &error, std::vector<std::string> &warnings)
{
    return {};
}

WalletCreationStatus IotaNode::createWallet(const SecureString &passphrase, uint64_t wallet_creation_flags, const std::string &name,
                                            std::string &error, std::vector<std::string> &warnings, std::unique_ptr<Wallet> &result)
{
    SecureString password(passphrase);
    create_account(name.data(), &password[0]);

    loadAccounts();

    UserAccount acc;
    if(tryGetUserAccount(QString::fromStdString(name), acc))
    {
        result = createIotaWallet(acc);
        return WalletCreationStatus::SUCCESS;
    }

    return WalletCreationStatus::FAILURE;
}

std::unique_ptr<Handler> IotaNode::handleInitMessage(Node::InitMessageFn fn)
{
    return MakeHandler({});
}

std::unique_ptr<Handler> IotaNode::handleMessageBox(Node::MessageBoxFn fn)
{
    return MakeHandler({});
}

std::unique_ptr<Handler> IotaNode::handleQuestion(Node::QuestionFn fn)
{
    return MakeHandler({});
}

std::unique_ptr<Handler> IotaNode::handleShowProgress(Node::ShowProgressFn fn)
{
    return MakeHandler({});
}

std::unique_ptr<Handler> IotaNode::handleLoadWallet(Node::LoadWalletFn fn)
{
    return MakeHandler({});
}

std::unique_ptr<Handler> IotaNode::handleNotifyNumConnectionsChanged(Node::NotifyNumConnectionsChangedFn fn)
{
    auto conn = connect(this, &IotaNode::connectionsNumChanged, this, [fn](int newConnectionsNum) {
        fn(newConnectionsNum);
    } );

    return MakeHandler([conn] { disconnect(conn);});
}

std::unique_ptr<Handler> IotaNode::handleNotifyNetworkActiveChanged(Node::NotifyNetworkActiveChangedFn fn)
{
    return MakeHandler({});
}

std::unique_ptr<Handler> IotaNode::handleNotifyAlertChanged(Node::NotifyAlertChangedFn fn)
{
    return MakeHandler({});
}

std::unique_ptr<Handler> IotaNode::handleBannedListChanged(Node::BannedListChangedFn fn)
{
    return MakeHandler({});
}

std::unique_ptr<Handler> IotaNode::handleNotifyBlockTip(Node::NotifyBlockTipFn fn)
{
    auto conn = connect(this, &IotaNode::latestMiltestoneChanged,
                        this, [fn](auto newMilestoneIndex, auto newMilestone) {
        fn(newMilestoneIndex, newMilestone.toStdString());
    });

    return MakeHandler([conn] { disconnect(conn); });
}

std::unique_ptr<Handler> IotaNode::handleNotifyAppInfochanged(NotifyAppInfoChangedFn fn)
{
    auto conn = connect(this, &IotaNode::appInfoChanged,
                         this, [fn](auto newAppName, auto newAppVersion){
        fn(newAppName.toStdString(), newAppVersion.toStdString());
    });

    return MakeHandler([conn] {disconnect(conn); });
}

std::unique_ptr<Handler> IotaNode::handleNotifyHeaderTip(Node::NotifyHeaderTipFn fn)
{
    return MakeHandler({});
}

void IotaNode::updateNodeStatus(QJsonObject nodeInfo)
{
    _nodeInfo = nodeInfo;
    setLatestMiltesone(qMakePair(_nodeInfo.value("latestMilestoneIndex").toInt(),
                                 _nodeInfo.value("latestMilestone").toString()));
    setAppInfo(qMakePair(_nodeInfo.value("appName").toString(), _nodeInfo.value("appVersion").toString()));
    setConnectionsNum(_nodeInfo.value("neighbors").toInt());

//    qDebug() << _nodeInfo;
}

void IotaNode::setLatestMiltesone(QPair<int, QString> newMilestone)
{
    if(newMilestone != _latestMilestone)
    {
        _latestMilestone = newMilestone;
        emit latestMiltestoneChanged(newMilestone.first,
                                     newMilestone.second);
    }
}

void IotaNode::setAppInfo(QPair<QString, QString> newAppInfo)
{
    if(newAppInfo != _appInfo)
    {
        _appInfo = newAppInfo;
        emit appInfoChanged(newAppInfo.first,
                            newAppInfo.second);
    }
}

void IotaNode::setConnectionsNum(int connectionsNum)
{
    if(connectionsNum != _connectionsNum)
    {
        _connectionsNum = connectionsNum;
        emit connectionsNumChanged(connectionsNum);
    }
}

void IotaNode::loadAccounts()
{
    _accounts.clear();
    c_string_unique_ptr accounts(get_accounts());
    for(auto val : QJsonDocument::fromJson(QByteArray(accounts.get())).array())
    {
        _accounts.emplace_back(UserAccount::FromJson(val.toObject()));
    }
}

void IotaNode::registerEvents()
{
    register_callback("node_updated", OnNodeUpdated);
    register_callback("balance_changed", OnBalanceChanged);
    register_callback("transaction_received", OnTransactionReceived);
    register_callback("transaction_sent", OnTransactionSent);
    register_callback("sent_transaction_confirmed", OnSentTransactionConfirmed);
    register_callback("transaction_received_confirmed", OnReceivedTransactionConfirmed);
}

bool IotaNode::tryGetUserAccount(const QString &username, UserAccount &account) const
{
    auto it = std::find_if(std::begin(_accounts), std::end(_accounts), [name = username](const auto &acc) {
        return acc.username == name;
    });

    if(it != std::end(_accounts))
    {
        account = *it;
        return true;
    }

    return false;
}

std::unique_ptr<Wallet> IotaNode::createIotaWallet(UserAccount account)
{
    auto wallet = std::make_unique<IotaWallet>(account);
    QPointer<IotaWallet> walletRaw(wallet.get());
    connect(this, &IotaNode::accountChanged, walletRaw, [walletRaw, this](QString username) {
        if(walletRaw && walletRaw->getWalletName() == username.toStdString())
        {
            UserAccount acc;
            if(this->tryGetUserAccount(username, acc))
            {
                walletRaw->onAccountUpdated(acc, _latestMilestone.first);
            }
        }
    });
    connect(this, &IotaNode::balanceChanged, walletRaw, [walletRaw, this](QString username, QString balance) {
        if(walletRaw && walletRaw->getWalletName() == username.toStdString())
        {
            UserAccount acc;
            if(this->tryGetUserAccount(username, acc))
            {
                walletRaw->onAccountBalanceUpdated(balance);
            }
        }
    });
    connect(this, &IotaNode::transactionChanged, walletRaw, [walletRaw, this](QString username, QJsonObject payload) {
        if(walletRaw && walletRaw->getWalletName() == username.toStdString())
        {
            UserAccount acc;
            if(this->tryGetUserAccount(username, acc))
            {
                walletRaw->onTransactionChanged(payload);
            }
        }
    });
    walletRaw->onAccountUpdated(account, _latestMilestone.first);
    return wallet;
}

//! Return implementation of Node interface.
std::unique_ptr<Node> MakeNode()
{
    auto node = std::make_unique<IotaNode>();
    _nodes.emplace_back(node.get());
    return node;
}

} // namespace interfaces
