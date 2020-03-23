// Copyright (c) 2018-2020 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

//#include <optional.h>               // For Optional and nullopt
//#include <primitives/transaction.h> // For CTransactionRef

#include <interfaces/chain.h>
#include <interfaces/handler.h>
#include <uint256.h>

namespace interfaces {

class IotaChain : public Chain
{
    // Chain interface
public:
    bool findBlock(const uint256 &hash, CBlock *block, int64_t *time, int64_t *max_time) override
    {
        return false;
    }
    double guessVerificationProgress(const uint256 &block_hash) override
    {
        return 0.0;
    }
    bool isReadyToBroadcast() override
    {
        return true;
    }
    bool isInitialBlockDownload() override
    {
        return false;
    }
    bool shutdownRequested() override
    {
        return false;
    }
    int64_t getAdjustedTime() override
    {
        return 0;
    }
    void initMessage(const std::string &message) override
    {
    }
    void initWarning(const std::string &message) override
    {
    }
    void initError(const std::string &message) override
    {
    }
    void showProgress(const std::string &title, int progress, bool resume_possible) override
    {
    }
    std::unique_ptr<Handler> handleNotifications(Notifications &notifications) override
    {
        return MakeHandler({});
    }

    void waitForNotificationsIfTipChanged(const uint256 &old_tip) override
    {
    }
    std::unique_ptr<Handler> handleRpc(const CRPCCommand &command) override
    {
        return MakeHandler({});
    }
    void rpcRunLater(const std::string &name, std::function<void ()> fn, int64_t seconds) override
    {
    }
};

class IotaChainClient : public ChainClient
{
public:
    void registerRpcs() override
    {
    }
    bool verify() override
    {
        return true;
    }
    bool load() override
    {
        return true;
    }
    void start(CScheduler &scheduler) override
    {
    }
    void flush() override
    {
    }
    void stop() override
    {
    }
};

//! Return implementation of Chain interface.
std::unique_ptr<Chain> MakeChain(NodeContext& node)
{
    return std::make_unique<IotaChain>();
}

//! Return implementation of ChainClient interface for a wallet client. This
//! function will be undefined in builds where ENABLE_WALLET is false.
//!
//! Currently, wallets are the only chain clients. But in the future, other
//! types of chain clients could be added, such as tools for monitoring,
//! analysis, or fee estimation. These clients need to expose their own
//! MakeXXXClient functions returning their implementations of the ChainClient
//! interface.
std::unique_ptr<ChainClient> MakeWalletClient(Chain& chain, std::vector<std::string> wallet_filenames)
{
    return std::make_unique<IotaChainClient>();
}

} // namespace interfaces
