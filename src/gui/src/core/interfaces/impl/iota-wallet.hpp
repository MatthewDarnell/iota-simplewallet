// Copyright (c) 2018-2019 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef IOTAWALLET_HPP
#define IOTAWALLET_HPP

#include <interfaces/wallet.h>
#include <interfaces/handler.h>

#include <QObject>
#include <QJsonObject>
#include <QDateTime>

struct FreeCStringDeleter
{
    void operator()(char *ptr)
    {
        free(ptr);
    }
};

using c_string_unique_ptr = std::unique_ptr<char, FreeCStringDeleter>;

namespace interfaces {

struct UserAccount
{
    int index { -1 };
    QString username;
    QString balance;
    bool synced;
    QDateTime createdAt;
    static UserAccount FromJson(QJsonObject obj);
};

class IotaWallet : public QObject, public Wallet
{
    // Wallet interface
    Q_OBJECT
public:
    explicit IotaWallet(UserAccount account,
                        QObject *parent = nullptr);

    bool encryptWallet(const SecureString &wallet_passphrase) override;
    bool isCrypted() override;
    bool lock() override;
    bool unlock(const SecureString &wallet_passphrase) override;
    bool isLocked() override;
    bool changeWalletPassphrase(const SecureString &old_wallet_passphrase, const SecureString &new_wallet_passphrase) override;
    void abortRescan() override;
    bool backupWallet(const std::string &filename) override;
    std::string getWalletName() override;
    bool getNewDestination(const std::string label, std::string &dest) override;
    bool getPubKey(const std::string &address, std::string &pub_key) override;
    bool getPrivKey(const std::string &address, std::string &key) override;
    bool isSpendable(const std::string &dest) override;
    bool setAddressBook(const std::string &dest, const std::string &name, const std::string &purpose) override;
    bool delAddressBook(const std::string &dest) override;
    bool getAddress(const std::string &dest, std::string *name, isminetype *is_mine, std::string *purpose) override;
    std::vector<WalletAddress> getAddresses() override;
    bool addDestData(const std::string &dest, const std::string &key, const std::string &value) override;
    bool eraseDestData(const std::string &dest, const std::string &key) override;
    std::vector<std::string> getDestValues(const std::string &prefix) override;
    bool generateAddresses(int count, std::string &fail_reason) override;
    WalletMutableTransaction createTransaction(const std::vector<CRecipient>& recipients, std::string& fail_reason) override;
    bool commitTransaction(WalletMutableTransaction tx,  WalletOrderForm order_form,  std::string &fail_reason) override;
    WalletTx getWalletTx(const std::string &txid) override;
    std::vector<WalletTx> getWalletTxs() override;
    uint32_t numberOfAddresses() override;
    bool tryGetTxStatus(const std::string &txid, WalletTxStatus &tx_status, int &num_blocks, int64_t &block_time) override;
    WalletTx getWalletTxDetails(const std::string &txid, WalletTxStatus &tx_status, WalletOrderForm &order_form, bool &in_mempool, int &num_blocks) override;
    WalletBalances getBalances() override;
    bool tryGetBalances(WalletBalances &balances, int &num_blocks) override;
    CAmount getBalance() override;
    CAmount getAvailableBalance() override;
    CAmount getRequiredFee(unsigned int tx_bytes) override;
    CAmount getMinimumFee(unsigned int tx_bytes, const CCoinControl &coin_control, int *returned_target, FeeReason *reason) override;
    unsigned int getConfirmTarget() override;
    bool hdEnabled() override;
    bool canGetAddresses() override;
    bool IsWalletFlagSet(uint64_t flag) override;
    void remove() override;
    std::unique_ptr<Handler> handleUnload(UnloadFn fn) override;
    std::unique_ptr<Handler> handleShowProgress(ShowProgressFn fn) override;
    std::unique_ptr<Handler> handleStatusChanged(StatusChangedFn fn) override;
    std::unique_ptr<Handler> handleAddressBookChanged(AddressBookChangedFn fn) override;
    std::unique_ptr<Handler> handleTransactionChanged(TransactionChangedFn fn) override;
    std::unique_ptr<Handler> handleBalanceChanged(BalanceChangedFn fn) override;
    std::unique_ptr<Handler> handleCanGetAddressesChanged(CanGetAddressesChangedFn fn) override;

signals:
    void transactionAdded(QString txid);
    void transactionUpdated(QString txid);
    void balanceChanged();

public slots:
    void onAccountUpdated(UserAccount account, int milestoneIndex);
    void onAccountBalanceUpdated(QString balance);
    void onTransactionChanged(QJsonObject payload);

private:
    void updateTransactions();
    void updateUnconfirmedBalance();

private:
    UserAccount _account;
    SecureString _unlockPassword;
    std::map<std::string, WalletTx> _transactions;
    CAmount _uncofirmedBalance { 0 };
    int _latestMilestoneIndex { 0 };
};

} // namespace interfaces

#endif
