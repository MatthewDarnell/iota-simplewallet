//
// Created by matth on 2/28/2020.
//

#ifndef IOTA_SIMPLEWALLET_TRANSACTION_H
#define IOTA_SIMPLEWALLET_TRANSACTION_H
char* get_incoming_transaction_by_hash(sqlite3* db, char* hash);
char* get_incoming_transactions(sqlite3* db, char* username, int offset, int num);
#endif //IOTA_SIMPLEWALLET_TRANSACTION_H
