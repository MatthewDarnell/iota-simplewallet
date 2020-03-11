//
// Created by matth on 2/18/2020.
//

#ifndef IOTA_SIMPLEWALLET_OUTGOING_TRANSACTION_H
#define IOTA_SIMPLEWALLET_OUTGOING_TRANSACTION_H
#include <stdint.h>
#include <sqlite3.h>
#include <cjson/cJSON.h>
int create_outgoing_transaction(sqlite3* db, const char* dest_address, const char* change_address, uint64_t amount, const char* trytes);
cJSON* get_outgoing_transaction_by_serial(sqlite3* db, int serial);
cJSON* get_outgoing_transaction_hash(sqlite3* db, char* hash);
cJSON* get_all_outgoing_transactions(sqlite3* db, const char* username, uint32_t offset, uint32_t limit);
cJSON* get_all_unsent_outgoing_transactions(sqlite3* db);
cJSON* get_all_unconfirmed_outgoing_transactions(sqlite3* db);
int mark_outgoing_transaction_confirmed(sqlite3* db, int serial, const char* bundle, const char* hash);
int mark_outgoing_transaction_sent(sqlite3* db, int serial, const char* bundle, const char* hash);
#endif //IOTA_SIMPLEWALLET_OUTGOING_TRANSACTION_H
