// Copyright (c) 2011-2019 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <transactionrecord.h>

#include <interfaces/wallet.h>

#include <stdint.h>
#include <tinyformat.h>

#include <QDateTime>
#include <QDebug>

/* Return positive answer if transaction should be shown in list.
 */
bool TransactionRecord::showTransaction()
{
    // There are currently no cases where we hide transactions, but
    // we may want to use this in the future for things like RBF.
    return true;
}

/*
 * Decompose CWallet transaction to model transaction records.
 */
QList<TransactionRecord> TransactionRecord::decomposeTransaction(const interfaces::WalletTx& wtx)
{

    TransactionRecord record;

    record.hash = wtx.hash;
    record.time = wtx.time;
    record.address = wtx.address;
    record.debit = wtx.debit;
    record.credit = wtx.credit;
    record.status.status = wtx.is_confirmed ? TransactionStatus::Confirmed : TransactionStatus::Unconfirmed;
    record.type = wtx.debit > 0 ? TransactionRecord::RecvWithAddress : TransactionRecord::SendToAddress;

    return {record};
}

void TransactionRecord::updateStatus(const interfaces::WalletTxStatus& wtx, int numBlocks, int64_t block_time)
{
    // Sort order, unrecorded transactions sort to the top
    status.sortKey = strprintf("%010d-%010u",
                               wtx.time_received,
                               wtx.is_confirmed);

    status.cur_num_blocks = numBlocks;
    status.status = wtx.is_confirmed ? TransactionStatus::Confirmed : TransactionStatus::Unconfirmed;
    status.needsUpdate = false;
}

bool TransactionRecord::statusUpdateNeeded(int numBlocks) const
{
    return status.cur_num_blocks != numBlocks || status.needsUpdate;
}

QString TransactionRecord::getTxHash() const
{
    return QString::fromStdString(hash);
}

