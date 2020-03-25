// Copyright (c) 2018-2019 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef BITCOIN_INTERFACES_NODE_H
#define BITCOIN_INTERFACES_NODE_H

#include <amount.h>     // For CAmount
#include <support/allocators/secure.h> // For SecureString

#include <functional>
#include <memory>
#include <stddef.h>
#include <stdint.h>
#include <string>
#include <tuple>
#include <vector>
#include <QString>

class BanMan;
class CCoinControl;
class CFeeRate;
class CNodeStats;
class Coin;
class RPCTimerInterface;
class proxyType;
struct CNodeStateStats;
struct NodeContext;

enum class WalletCreationStatus {
    SUCCESS,
    FAILURE
};

namespace interfaces {
class Handler;
class Wallet;

//! Top-level interface for a bitcoin node (bitcoind process).
class Node
{
public:
    virtual ~Node() {}

    //! Send init error.
    virtual void initError(const std::string& message) = 0;

    //! Set command line arguments.
    virtual bool parseParameters(int argc, const char* const argv[], std::string& error) = 0;

    //! Load settings from configuration file.
    virtual bool readConfigFiles(std::string& error) = 0;

    //! Choose network parameters.
    virtual void selectParams(const std::string& network) = 0;

    //! Get the (assumed) blockchain size.
    virtual uint64_t getAssumedBlockchainSize() = 0;

    //! Get the (assumed) chain state size.
    virtual uint64_t getAssumedChainStateSize() = 0;

    //! Get connected node name.
    virtual QString getNetwork() = 0;

    virtual std::string getDataDir() = 0;

    //! Init logging.
    virtual void initLogging() = 0;

    //! Init parameter interaction.
    virtual void initParameterInteraction() = 0;

    //! Get warnings.
    virtual std::string getWarnings() = 0;

    // Get log flags.
    virtual uint32_t getLogCategories() = 0;

    //! Initialize app dependencies.
    virtual bool baseInitialize() = 0;

    //! Start node.
    virtual bool appInitMain() = 0;

    //! Stop node.
    virtual void appShutdown() = 0;

    //! Start shutdown.
    virtual void startShutdown() = 0;

    //! Return whether shutdown was requested.
    virtual bool shutdownRequested() = 0;

    //! Setup arguments
    virtual void setupServerArgs() = 0;

    //! Get header tip height and time.
    virtual bool getHeaderTip(int& height, int64_t& block_time) = 0;

    //! Get num blocks.
    virtual int getNumBlocks(bool solid) = 0;

    //! Get last block time.
    virtual int64_t getLastBlockTime() = 0;

    //! Get verification progress.
    virtual double getVerificationProgress() = 0;

    //! Is initial block download.
    virtual bool isInitialBlockDownload() = 0;

    //! Get reindex.
    virtual bool getReindex() = 0;

    //! Get importing.
    virtual bool getImporting() = 0;

    //! Set network active.
    virtual void setNetworkActive(bool active) = 0;

    //! Get network active.
    virtual bool getNetworkActive() = 0;

    //! Get latest milestone
    virtual QString getLatestMilestone(bool solid) = 0;

    //! Get application name
    virtual QString getAppName() = 0;

    //! Get aplication version
    virtual QString getAppVersion() = 0;

//    //! Estimate smart fee.
//    virtual CFeeRate estimateSmartFee(int num_blocks, bool conservative, int* returned_target = nullptr) = 0;

//    //! Get dust relay fee.
//    virtual CFeeRate getDustRelayFee() = 0;

    //! Execute rpc command.
    virtual std::string executeRpc(const std::string& command, const std::vector<std::string> params, const std::string &uri) = 0;

    //! List rpc commands.
    virtual std::vector<std::string> listRpcCommands() = 0;

    //! Set RPC timer interface if unset.
    virtual void rpcSetTimerInterfaceIfUnset(RPCTimerInterface* iface) = 0;

    //! Unset RPC timer interface.
    virtual void rpcUnsetTimerInterface(RPCTimerInterface* iface) = 0;

    //! Return default wallet directory.
    virtual std::string getWalletDir() = 0;

    //! Return available wallets in wallet directory.
    virtual std::vector<std::string> listWalletDir() = 0;

    //! Return interfaces for accessing wallets (if any).
    virtual std::vector<std::unique_ptr<Wallet>> getWallets() = 0;

    //! Attempts to load a wallet from file or directory.
    //! The loaded wallet is also notified to handlers previously registered
    //! with handleLoadWallet.
    virtual std::unique_ptr<Wallet> loadWallet(const std::string &username, const SecureString& passphrase, const SecureString& seed, std::string& error) = 0;

    //! Create a wallet from file
    virtual WalletCreationStatus createWallet(const SecureString& passphrase, uint64_t wallet_creation_flags, const std::string& name, std::string& error, std::vector<std::string>& warnings, std::unique_ptr<Wallet>& result) = 0;

    //! Register handler for init messages.
    using InitMessageFn = std::function<void(const std::string& message)>;
    virtual std::unique_ptr<Handler> handleInitMessage(InitMessageFn fn) = 0;

    //! Register handler for message box messages.
    using MessageBoxFn =
        std::function<bool(const std::string& message, const std::string& caption, unsigned int style)>;
    virtual std::unique_ptr<Handler> handleMessageBox(MessageBoxFn fn) = 0;

    //! Register handler for question messages.
    using QuestionFn = std::function<bool(const std::string& message,
        const std::string& non_interactive_message,
        const std::string& caption,
        unsigned int style)>;
    virtual std::unique_ptr<Handler> handleQuestion(QuestionFn fn) = 0;

    //! Register handler for progress messages.
    using ShowProgressFn = std::function<void(const std::string& title, int progress, bool resume_possible)>;
    virtual std::unique_ptr<Handler> handleShowProgress(ShowProgressFn fn) = 0;

    //! Register handler for load wallet messages.
    using LoadWalletFn = std::function<void(std::unique_ptr<Wallet> wallet)>;
    virtual std::unique_ptr<Handler> handleLoadWallet(LoadWalletFn fn) = 0;

    //! Register handler for number of connections changed messages.
    using NotifyNumConnectionsChangedFn = std::function<void(int new_num_connections)>;
    virtual std::unique_ptr<Handler> handleNotifyNumConnectionsChanged(NotifyNumConnectionsChangedFn fn) = 0;

    //! Register handler for network active messages.
    using NotifyNetworkActiveChangedFn = std::function<void(bool network_active)>;
    virtual std::unique_ptr<Handler> handleNotifyNetworkActiveChanged(NotifyNetworkActiveChangedFn fn) = 0;

    //! Register handler for notify alert messages.
    using NotifyAlertChangedFn = std::function<void()>;
    virtual std::unique_ptr<Handler> handleNotifyAlertChanged(NotifyAlertChangedFn fn) = 0;

    //! Register handler for ban list messages.
    using BannedListChangedFn = std::function<void()>;
    virtual std::unique_ptr<Handler> handleBannedListChanged(BannedListChangedFn fn) = 0;

    //! Register handler for block tip messages.
    using NotifyBlockTipFn =
        std::function<void(int height, std::string newBlockTip)>;
    virtual std::unique_ptr<Handler> handleNotifyBlockTip(NotifyBlockTipFn fn) = 0;

    //! Register handler for header tip messages.
    using NotifyHeaderTipFn =
        std::function<void(bool initial_download, int height, int64_t block_time, double verification_progress)>;
    virtual std::unique_ptr<Handler> handleNotifyHeaderTip(NotifyHeaderTipFn fn) = 0;

    //!Register handler for app info messages
    using NotifyAppInfoChangedFn = std::function<void(std::string appName, std::string appVersion, std::string connectedNode)>;
    virtual std::unique_ptr<Handler> handleNotifyAppInfochanged(NotifyAppInfoChangedFn fn) = 0;

    //! Return pointer to internal chain interface, useful for testing.
    virtual NodeContext* context() { return nullptr; }
};

//! Return implementation of Node interface.
std::unique_ptr<Node> MakeNode();

} // namespace interfaces

#endif // BITCOIN_INTERFACES_NODE_H
