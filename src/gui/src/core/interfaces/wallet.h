// Copyright (c) 2018-2019 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef BITCOIN_INTERFACES_WALLET_H
#define BITCOIN_INTERFACES_WALLET_H

#include <amount.h>                    // For CAmount
//#include <pubkey.h>                    // For std::string and CScriptID (definitions needed in std::string instantiation)
//#include <script/standard.h>           // For std::string
#include <support/allocators/secure.h> // For SecureString
//    // For ChangeType
#include <uint256.h>

#include <functional>
#include <map>
#include <memory>
//#include <psbt.h>
#include <stdint.h>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

class CCoinControl;
class CFeeRate;
class CWallet;
enum isminetype : unsigned int;
enum class FeeReason;
enum ChangeType : unsigned int;
typedef uint8_t isminefilter;

namespace interfaces {

class Handler;
struct CRecipient;
struct WalletAddress;
struct WalletBalances;
struct WalletTx;
struct WalletTxOut;
struct WalletTxStatus;
struct WalletMutableTransaction;

using WalletOrderForm = std::vector<std::pair<std::string, std::string>>;
using WalletValueMap = std::map<std::string, std::string>;

//! Interface for accessing a wallet.
class Wallet
{
public:
    virtual ~Wallet() {}

    //! Encrypt wallet.
    virtual bool encryptWallet(const SecureString& wallet_passphrase) = 0;

    //! Return whether wallet is encrypted.
    virtual bool isCrypted() = 0;

    //! Lock wallet.
    virtual bool lock() = 0;

    //! Unlock wallet.
    virtual bool unlock(const SecureString& wallet_passphrase) = 0;

    //! Return whether wallet is locked.
    virtual bool isLocked() = 0;

    //! Change wallet passphrase.
    virtual bool changeWalletPassphrase(const SecureString& old_wallet_passphrase,
                                        const SecureString& new_wallet_passphrase) = 0;

    //! Abort a rescan.
    virtual void abortRescan() = 0;

    //! Back up wallet.
    virtual bool backupWallet(const std::string& filename) = 0;

    //! Get wallet name.
    virtual std::string getWalletName() = 0;

    // Get a new address.
    virtual bool getNewDestination(const std::string label, std::string& dest) = 0;

    //! Get public key.
    virtual bool getPubKey(const std::string& address, std::string& pub_key) = 0;

    //! Get private key.
    virtual bool getPrivKey(const std::string& address, std::string& key) = 0;

    //! Return whether wallet has private key.
    virtual bool isSpendable(const std::string& dest) = 0;

    //! Add or update address.
    virtual bool setAddressBook(const std::string& dest, const std::string& name, const std::string& purpose) = 0;

    // Remove address.
    virtual bool delAddressBook(const std::string& dest) = 0;

    //! Look up address in wallet, return whether exists.
    virtual bool getAddress(const std::string& dest,
                            std::string* name,
                            isminetype* is_mine,
                            std::string* purpose) = 0;

    //! Get wallet address list.
    virtual std::vector<WalletAddress> getAddresses() = 0;

    //! Add dest data.
    virtual bool addDestData(const std::string& dest, const std::string& key, const std::string& value) = 0;

    //! Erase dest data.
    virtual bool eraseDestData(const std::string& dest, const std::string& key) = 0;

    //! Get dest values with prefix.
    virtual std::vector<std::string> getDestValues(const std::string& prefix) = 0;

    virtual bool generateAddresses(int count, std::string &fail_reason) = 0;

    //! Create transaction.
    virtual WalletMutableTransaction createTransaction(const std::vector<CRecipient>& recipients,
                                                       std::string& fail_reason) = 0;

    //! Commit transaction.
    virtual bool commitTransaction(WalletMutableTransaction tx,
                                   WalletOrderForm order_form,
                                   std::string &fail_reason) = 0;

    virtual uint32_t numberOfAddresses() = 0;

    //! Get transaction information.
    virtual WalletTx getWalletTx(const std::string& txid) = 0;

    //! Get list of all wallet transactions.
    virtual std::vector<WalletTx> getWalletTxs() = 0;

    //! Try to get updated status for a particular transaction, if possible without blocking.
    virtual bool tryGetTxStatus(const std::string& txid,
                                WalletTxStatus& tx_status,
                                int& num_blocks,
                                int64_t& block_time) = 0;

    //! Get transaction details.
    virtual WalletTx getWalletTxDetails(const std::string& txid,
                                        WalletTxStatus& tx_status,
                                        WalletOrderForm& order_form,
                                        bool& in_mempool,
                                        int& num_blocks) = 0;

    //! Get balances.
    virtual WalletBalances getBalances() = 0;

    //! Get balances if possible without blocking.
    virtual bool tryGetBalances(WalletBalances& balances, int& num_blocks) = 0;

    //! Get balance.
    virtual CAmount getBalance() = 0;

    //! Get available balance.
    virtual CAmount getAvailableBalance() = 0;

    //! Return whether transaction input belongs to wallet.
    //    virtual isminetype txinIsMine(const CTxIn& txin) = 0;

    //    //! Return whether transaction output belongs to wallet.
    //    virtual isminetype txoutIsMine(const CTxOut& txout) = 0;

    //    //! Return debit amount if transaction input belongs to wallet.
    //    virtual CAmount getDebit(const CTxIn& txin, isminefilter filter) = 0;

    //    //! Return credit amount if transaction input belongs to wallet.
    //    virtual CAmount getCredit(const CTxOut& txout, isminefilter filter) = 0;

    //    //! Return AvailableCoins + LockedCoins grouped by wallet address.
    //    //! (put change in one group with wallet address)
    //    using CoinsList = std::map<std::string, std::vector<std::tuple<COutPoint, WalletTxOut>>>;
    //    virtual CoinsList listCoins() = 0;

    //    //! Return wallet transaction output information.
    //    virtual std::vector<WalletTxOut> getCoins(const std::vector<COutPoint>& outputs) = 0;

    //! Get required fee.
    virtual CAmount getRequiredFee(unsigned int tx_bytes) = 0;

    //! Get minimum fee.
    virtual CAmount getMinimumFee(unsigned int tx_bytes,
                                  const CCoinControl& coin_control,
                                  int* returned_target,
                                  FeeReason* reason) = 0;

    //! Get tx confirm target.
    virtual unsigned int getConfirmTarget() = 0;

    // Return whether HD enabled.
    virtual bool hdEnabled() = 0;

    // Return whether the wallet is blank.
    virtual bool canGetAddresses() = 0;

    // check if a certain wallet flag is set.
    virtual bool IsWalletFlagSet(uint64_t flag) = 0;

    // Remove wallet.
    virtual void remove() = 0;

    //! Register handler for unload message.
    using UnloadFn = std::function<void()>;
    virtual std::unique_ptr<Handler> handleUnload(UnloadFn fn) = 0;

    //! Register handler for show progress messages.
    using ShowProgressFn = std::function<void(const std::string& title, int progress)>;
    virtual std::unique_ptr<Handler> handleShowProgress(ShowProgressFn fn) = 0;

    //! Register handler for status changed messages.
    using StatusChangedFn = std::function<void()>;
    virtual std::unique_ptr<Handler> handleStatusChanged(StatusChangedFn fn) = 0;

    //! Register handler for address book changed messages.
    using AddressBookChangedFn = std::function<void(const std::string& address,
    const std::string& label,
    bool is_mine,
    const std::string& purpose)>;
    virtual std::unique_ptr<Handler> handleAddressBookChanged(AddressBookChangedFn fn) = 0;

    //! Register handler for transaction changed messages.
    using TransactionChangedFn = std::function<void(const std::string& txid, const ChangeType &changeType)>;
    virtual std::unique_ptr<Handler> handleTransactionChanged(TransactionChangedFn fn) = 0;

    //! Register handler for keypool changed messages.
    using CanGetAddressesChangedFn = std::function<void()>;
    virtual std::unique_ptr<Handler> handleCanGetAddressesChanged(CanGetAddressesChangedFn fn) = 0;

    using BalanceChangedFn = std::function<void(void)>;
    virtual std::unique_ptr<Handler> handleBalanceChanged(BalanceChangedFn fn) = 0;
};

//! Information about one wallet address.
struct WalletAddress
{
    std::string dest;
    isminetype is_mine;
    std::string name;
    std::string purpose;

    WalletAddress(std::string dest, isminetype is_mine, std::string name, std::string purpose)
        : dest(std::move(dest)), is_mine(is_mine), name(std::move(name)), purpose(std::move(purpose))
    {
    }
};

struct CRecipient
{
    std::string dest;
    CAmount amount = 0;
};

struct WalletMutableTransaction
{
    CRecipient recipient;
    WalletValueMap value_map;
};

//! Collection of wallet balances.
struct WalletBalances
{
    CAmount balance = 0;
    CAmount unconfirmed_balance = 0;
    CAmount immature_balance = 0;
    bool have_watch_only = false;
    CAmount watch_only_balance = 0;
    CAmount unconfirmed_watch_only_balance = 0;
    CAmount immature_watch_only_balance = 0;

    bool balanceChanged(const WalletBalances& prev) const
    {
        return balance != prev.balance || unconfirmed_balance != prev.unconfirmed_balance ||
                immature_balance != prev.immature_balance || watch_only_balance != prev.watch_only_balance ||
                unconfirmed_watch_only_balance != prev.unconfirmed_watch_only_balance ||
                immature_watch_only_balance != prev.immature_watch_only_balance;
    }
};

// Wallet transaction information.
struct WalletTx
{
    std::string address;
    std::string hash;
    std::string bundle;
    CAmount credit { 0 };
    CAmount debit { 0 };
    int64_t time { 0 };
    std::map<std::string, std::string> value_map;
    bool is_confirmed { 0 };
};

//! Updated transaction status.
struct WalletTxStatus
{
    unsigned int time_received;
    bool is_confirmed { false };
};

} // namespace interfaces

#endif // BITCOIN_INTERFACES_WALLET_H
