// Copyright (c) 2018-2019 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <interfaces/node.h>
#include <interfaces/handler.h>
#include <interfaces/wallet.h>
#include "iota-wallet.hpp"

#include <QObject>
#include <QJsonObject>
#include <QDateTime>
#include <QTimer>
#include <QPair>

namespace interfaces {

class IotaNode : public QObject, public Node
{
    // Node interface
    Q_OBJECT
public:
    void initError(const std::string &message) override;
    bool parseParameters(int argc, const char * const argv[], std::string &error) override;
    bool readConfigFiles(std::string &error) override;
    void selectParams(const std::string &network) override;
    uint64_t getAssumedBlockchainSize() override;
    uint64_t getAssumedChainStateSize() override;
    QString getNetwork() override;
    std::string getDataDir() override;
    void initLogging() override;
    void initParameterInteraction() override;
    std::string getWarnings() override;
    uint32_t getLogCategories() override;
    bool baseInitialize() override;
    bool appInitMain() override;
    void appShutdown() override;
    void startShutdown() override;
    bool shutdownRequested() override;
    void setupServerArgs() override;;
    bool getHeaderTip(int &height, int64_t &block_time) override;
    int getNumBlocks(bool solid) override;
    int64_t getLastBlockTime() override;
    double getVerificationProgress() override;
    bool isInitialBlockDownload() override;
    bool getReindex() override;
    bool getImporting() override;
    void setNetworkActive(bool active) override;
    bool getNetworkActive() override;
    QString getAppName() override;
    QString getAppVersion() override;

    QString getLatestMilestone(bool solid) override;

    std::string executeRpc(const std::string& command, const std::vector<std::string> params,
                           const std::string &uri) override;
    std::vector<std::string> listRpcCommands() override;
    void rpcSetTimerInterfaceIfUnset(RPCTimerInterface *iface) override;
    void rpcUnsetTimerInterface(RPCTimerInterface *iface) override;
    std::string getWalletDir() override;
    std::vector<std::string> listWalletDir() override;
    std::vector<std::unique_ptr<Wallet> > getWallets() override;
    std::unique_ptr<Wallet> loadWallet(const std::string &username, const SecureString& passphrase, const SecureString& seed, std::string& error) override;
    std::unique_ptr<Wallet> loadWallet(const std::string &username, const SecureString& passphrase, const std::string& path, std::string& error) override;
    WalletCreationStatus createWallet(const SecureString &passphrase, uint64_t wallet_creation_flags, const std::string &name, std::string &error, std::vector<std::string> &warnings, std::unique_ptr<Wallet> &result) override;
    std::unique_ptr<Handler> handleInitMessage(InitMessageFn fn) override;
    std::unique_ptr<Handler> handleMessageBox(MessageBoxFn fn) override;
    std::unique_ptr<Handler> handleQuestion(QuestionFn fn) override;
    std::unique_ptr<Handler> handleShowProgress(ShowProgressFn fn) override;
    std::unique_ptr<Handler> handleLoadWallet(LoadWalletFn fn) override;
    std::unique_ptr<Handler> handleNotifyNumConnectionsChanged(NotifyNumConnectionsChangedFn fn) override;
    std::unique_ptr<Handler> handleNotifyNetworkActiveChanged(NotifyNetworkActiveChangedFn fn) override;
    std::unique_ptr<Handler> handleNotifyAlertChanged(NotifyAlertChangedFn fn) override;
    std::unique_ptr<Handler> handleBannedListChanged(BannedListChangedFn fn) override;
    std::unique_ptr<Handler> handleNotifyBlockTip(NotifyBlockTipFn fn) override;
    std::unique_ptr<Handler> handleNotifyHeaderTip(NotifyHeaderTipFn fn) override;
    std::unique_ptr<Handler> handleNotifyAppInfochanged(NotifyAppInfoChangedFn fn) override;


signals:
    void latestMiltestoneChanged(int newMilestoneIndex,
                                 QString newMilestone);
    void connectionsNumChanged(int connectionsNum);
    void appInfoChanged(QString appName,
                        QString appVersion,
                        QString connectedNode);
    void accountChanged(QString username);
    void balanceChanged(QString username, QString balance);
    void transactionChanged(QString username, QJsonObject payload);

private:
    void updateNodeStatus(QJsonObject nodeInfo);
    void setLatestMiltesone(QPair<int, QString> newMilestone);
    void setAppInfo(std::tuple<QString, QString, QString> newAppInfo);
    void setConnectionsNum(int connectionsNum);
    void loadAccounts();
    void registerEvents();
    bool tryGetUserAccount(const QString &username, UserAccount &account) const;
    std::unique_ptr<Wallet> createIotaWallet(UserAccount account);

    static void* OnBalanceChanged(const char *payload);
    static void* OnNodeUpdated(const char *payload);
    static void* OnTransactionReceived(const char *payload);
    static void* OnTransactionSent(const char *payload);
    static void* OnSentTransactionConfirmed(const char *payload);
    static void* OnReceivedTransactionConfirmed(const char *payload);

private:
    std::vector<UserAccount> _accounts;
    QJsonObject _nodeInfo;
    QPair<int, QString> _latestSolidMilestone;
    QPair<int, QString> _latestMilestone;
    int _numberOfConnections { 0 };
    std::tuple<QString, QString, QString> _appInfo;

    int _connectionsNum;
};

} // namespace interfaces
