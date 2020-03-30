// Copyright (c) 2018-2019 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "iota-wallet.hpp"
#include <iota/iota-simplewallet.h>
#include <ui-interface.h>
#include <util/strencodings.h>
#include <QJsonDocument>
#include <QJsonArray>
#include <QDebug>

namespace interfaces {

static bool IsValidTransaction(QJsonObject obj)
{
    return obj.contains("amount") &&
            obj.value("amount").toString().toLongLong() > 0;
}

static WalletTx ParseTransaction(QJsonObject obj)
{
    WalletTx tx;
    tx.hash = obj.value("hash").toString().toStdString();
    tx.time = obj.value("time").toString().toLongLong();
    tx.is_confirmed = obj.value("confirmed").toInt() > 0;
    tx.bundle = obj.value("bundle").toString().toStdString();
    tx.address = obj.value("address").toString().toStdString();
    auto amount = obj.value("amount").toString().toLongLong();;
    if(obj.contains("sent"))
    {
        tx.credit = -amount;
    }
    else
    {
        tx.debit = amount;
    }

    return tx;
}

static std::vector<WalletTx> FetchAllTransactionsHelper(std::function<c_string_unique_ptr(int, int)> fetcher)
{
    std::vector<WalletTx> result;
    static const uint32_t limit = 100;
    std::function<void(int)> worker = [&result, &worker, &fetcher](int offset) {
        auto txns = QJsonDocument::fromJson(QByteArray(fetcher(offset, limit).get())).array();

        if(txns.isEmpty())
        {
            return;
        }

        for(auto val : txns)
        {
            auto obj = val.toObject();
            if(IsValidTransaction(obj))
            {
                result.emplace_back(ParseTransaction(obj));
            }
        }

        worker(offset + txns.size());
    };

    worker(0);

    return result;
}

UserAccount UserAccount::FromJson(QJsonObject obj)
{
    UserAccount acc;
    acc.index = obj.value("index").toInt();
    acc.username = obj.value("username").toString();
    acc.balance = obj.value("balance").toString();
    acc.synced = obj.value("is_sycned").toInt() > 0;
    acc.createdAt = QDateTime::fromString(obj.value("created_at").toString());
    return acc;
}


IotaWallet::IotaWallet(UserAccount account, QObject *parent) :
    QObject(parent),
    _account(account)
{
    updateTransactions();
}

bool IotaWallet::encryptWallet(const SecureString &wallet_passphrase)
{
    return false;
}

bool IotaWallet::isCrypted()
{
    return true;
}

bool IotaWallet::lock()
{
    SecureString{}.swap(_unlockPassword);
    return true;
}

bool IotaWallet::unlock(const SecureString &wallet_passphrase)
{
    SecureString tmpPassword = wallet_passphrase;
    auto ret = verify_login(_account.username.toStdString().data(), &tmpPassword[0], 1);
    if(ret == 0)
    {
        _unlockPassword = wallet_passphrase;
        return true;
    }

    return false;
}

bool IotaWallet::isLocked()
{
    return _unlockPassword.empty();
}

bool IotaWallet::changeWalletPassphrase(const SecureString &old_wallet_passphrase, const SecureString &new_wallet_passphrase)
{
    return false;
}

void IotaWallet::abortRescan()
{
}

bool IotaWallet::backupWallet(const std::string &filename)
{
    if(isLocked())
    {
        return false;
    }

    auto password = _unlockPassword;
    return export_account_state(_account.username.toStdString().data(), &password[0], filename.c_str()) == 0;
}

std::string IotaWallet::getWalletName()
{
    return _account.username.toStdString();
}

bool IotaWallet::getNewDestination(const std::string label, std::string &dest)
{
    auto username = _account.username.toStdString();
    c_string_unique_ptr address(get_new_address(username.data()));
    auto obj = QJsonDocument::fromJson(QByteArray(address.get())).object();
    dest = obj.value("address").toString().toStdString();
    return !dest.empty();
}

bool IotaWallet::getPubKey(const std::string &address, std::string &pub_key)
{
    return false;
}

bool IotaWallet::getPrivKey(const std::string &address, std::string &key)
{
    return false;
}

bool IotaWallet::isSpendable(const std::string &dest)
{
    return false;
}

bool IotaWallet::setAddressBook(const std::string &dest, const std::string &name, const std::string &purpose)
{
    return false;
}

bool IotaWallet::delAddressBook(const std::string &dest)
{
    return false;
}

bool IotaWallet::getAddress(const std::string &dest, std::string *name, isminetype *is_mine, std::string *purpose)
{
    return false;
}

std::vector<WalletAddress> IotaWallet::getAddresses()
{
    return {};
}

bool IotaWallet::addDestData(const std::string &dest, const std::string &key, const std::string &value)
{
    Q_UNUSED(dest)
    return write_user_data(_account.username.toStdString().data(), key.data(), HexStr(value).data()) == 0;
}

bool IotaWallet::eraseDestData(const std::string &dest, const std::string &key)
{
    Q_UNUSED(dest)
    return delete_user_data(_account.username.toStdString().data(), key.data()) == 0;
}

std::vector<std::string> IotaWallet::getDestValues(const std::string &prefix)
{
    c_string_unique_ptr result(read_user_data(_account.username.toStdString().data(), prefix.data()));
    std::vector<std::string> data;
    if (result) {
        for (auto val : QJsonDocument::fromJson(QByteArray(result.get())).array()) {
            auto obj = val.toObject();
            auto v = ParseHex(obj.value("value").toString().toStdString());
            data.emplace_back(v.begin(), v.end());
        }
    }
    return data;
}

bool IotaWallet::generateAddresses(int count, std::string &fail_reason)
{
    if(isLocked())
    {
        fail_reason = "Wallet locked";
        return false;
    }

    SecureString password(_unlockPassword);
    auto r = generate_num_addresses(_account.username.toStdString().data(), &password[0], count);

    if (r < 0) {
        fail_reason = "Address generation failed";
        return false;
    }

    return true;
}

WalletMutableTransaction IotaWallet::createTransaction(const std::vector<CRecipient> &recipients, std::string &fail_reason)
{
    if(recipients.size() == 1)
    {
        WalletMutableTransaction tx;
        tx.recipient = recipients.front();
        fail_reason.clear();
        return tx;
    }

    fail_reason = "Unsupported recipient number";
    return {};
}

bool IotaWallet::commitTransaction(WalletMutableTransaction tx, WalletOrderForm order_form, std::string &fail_reason)
{
    if(isLocked())
    {
        fail_reason = "Wallet locked";
        return false;
    }

    SecureString password = _unlockPassword;
    auto r = create_transaction(_account.username.toStdString().data(), &password[0], tx.recipient.dest.data(), tx.recipient.amount);
    if(r == 0)
    {
        fail_reason.clear();
        return true;
    }

    fail_reason = "Failed to commit transaction: " + std::to_string(r);
    return false;
}

WalletTx IotaWallet::getWalletTx(const std::string &txid)
{
    return _transactions.count(txid) > 0 ? _transactions.at(txid) : WalletTx{};
}

std::vector<WalletTx> IotaWallet::getWalletTxs()
{
    std::vector<WalletTx> res;
    for(auto it : _transactions)
    {
        res.emplace_back(it.second);
    }

    return res;
}

bool IotaWallet::tryGetTxStatus(const std::string &txid, WalletTxStatus &tx_status, int &num_blocks, int64_t &block_time)
{
    if(_transactions.count(txid) > 0)
    {
        auto wtx = _transactions.at(txid);
        tx_status.time_received = wtx.time;
        tx_status.is_confirmed = wtx.is_confirmed;
        num_blocks = _latestMilestoneIndex;
        block_time = 0;
        return true;
    }

    return false;
}

WalletTx IotaWallet::getWalletTxDetails(const uint256 &txid, WalletTxStatus &tx_status, WalletOrderForm &order_form, bool &in_mempool, int &num_blocks)
{
    return {};
}

WalletBalances IotaWallet::getBalances()
{
    WalletBalances balances;
    balances.have_watch_only = false;
    balances.balance = _account.balance.toLongLong();
    balances.unconfirmed_balance = _uncofirmedBalance;
    balances.immature_balance = 0;

    return balances;
}

bool IotaWallet::tryGetBalances(WalletBalances &balances, int &num_blocks)
{
    balances = getBalances();
    num_blocks = _latestMilestoneIndex;
    return true;
}

CAmount IotaWallet::getBalance()
{
    return 0;
}

CAmount IotaWallet::getAvailableBalance()
{
    return _account.balance.toLongLong();
}

CAmount IotaWallet::getRequiredFee(unsigned int tx_bytes)
{
    return 0;
}

CAmount IotaWallet::getMinimumFee(unsigned int tx_bytes, const CCoinControl &coin_control, int *returned_target, FeeReason *reason)
{
    return 0;
}

unsigned int IotaWallet::getConfirmTarget()
{
    return 0;
}

bool IotaWallet::hdEnabled()
{
    return true;
}

bool IotaWallet::canGetAddresses()
{
    return true;
}

bool IotaWallet::IsWalletFlagSet(uint64_t flag)
{
    return false;
}

void IotaWallet::remove()
{
}

std::unique_ptr<Handler> IotaWallet::handleUnload(Wallet::UnloadFn fn)
{
    return MakeHandler({});
}

std::unique_ptr<Handler> IotaWallet::handleShowProgress(Wallet::ShowProgressFn fn)
{
    return MakeHandler({});
}

std::unique_ptr<Handler> IotaWallet::handleStatusChanged(Wallet::StatusChangedFn fn)
{
    return MakeHandler({});
}

std::unique_ptr<Handler> IotaWallet::handleAddressBookChanged(Wallet::AddressBookChangedFn fn)
{
    return MakeHandler({});
}

std::unique_ptr<Handler> IotaWallet::handleTransactionChanged(Wallet::TransactionChangedFn fn)
{
    std::vector<QMetaObject::Connection> connections;
    connections.emplace_back(connect(this, &IotaWallet::transactionAdded, this, [fn](QString txid) {
        fn(txid.toStdString(), ChangeType::CT_NEW);
    }));
    connections.emplace_back(connect(this, &IotaWallet::transactionUpdated, this, [fn](QString txid) {
        fn(txid.toStdString(), ChangeType::CT_UPDATED);
    }));

    return MakeHandler([connections] {
        for(auto &&conn : connections)
        {
            disconnect(conn);
        }
    });
}

std::unique_ptr<Handler> IotaWallet::handleBalanceChanged(Wallet::BalanceChangedFn fn)
{
    auto conn = connect(this, &IotaWallet::balanceChanged, this, fn);
    return MakeHandler([conn] {
        QObject::disconnect(conn);
    });
}

std::unique_ptr<Handler> IotaWallet::handleCanGetAddressesChanged(Wallet::CanGetAddressesChangedFn fn)
{
    return MakeHandler({});
}

void IotaWallet::onAccountUpdated(UserAccount account, int milestoneIndex)
{
    _account = account;
    _latestMilestoneIndex = milestoneIndex;
}

void IotaWallet::onAccountBalanceUpdated(QString balance)
{
    if(_account.balance != balance)
    {
        _account.balance = balance;
        Q_EMIT balanceChanged();
    }
}

void IotaWallet::onTransactionChanged(QJsonObject payload)
{
    if(!IsValidTransaction(payload))
    {
        return;
    }

    auto wtx = ParseTransaction(payload);
    auto idx = QString::fromStdString(wtx.hash);
    if(_transactions.count(wtx.hash) > 0)
    {
        _transactions.at(wtx.hash) = wtx;
        updateUnconfirmedBalance();
        Q_EMIT transactionUpdated(idx);
    }
    else
    {
        _transactions.emplace(wtx.hash, wtx);
        if(!wtx.is_confirmed)
        {
            updateUnconfirmedBalance();
        }
        Q_EMIT transactionAdded(idx);
    }
}

void IotaWallet::updateTransactions()
{
    auto username = _account.username.toStdString();
    auto usernameRaw = username.data();
    auto incoming_txns = FetchAllTransactionsHelper([usernameRaw](auto offset, auto limit) {
        return c_string_unique_ptr(get_incoming_transactions(usernameRaw, offset, limit));
    });
    auto outgoing_txns = FetchAllTransactionsHelper([usernameRaw](auto offset, auto limit) {
        return c_string_unique_ptr(get_outgoing_transactions(usernameRaw, offset, limit));
    });

    auto cpy = [this](const auto &from) {
        for(auto &&val : from)
        {
            _transactions[val.hash] = val;
        }
    };

    cpy(incoming_txns);
    cpy(outgoing_txns);

    updateUnconfirmedBalance();
}

void IotaWallet::updateUnconfirmedBalance()
{
    _uncofirmedBalance = 0;
    for (auto &&tx : _transactions)
    {
        if (!tx.second.is_confirmed)
        {
            _uncofirmedBalance += tx.second.credit + tx.second.debit;
        }
    }
}
} // namespace interfaces
