//
// Created by matth on 2/18/2020.
//
#ifndef IOTA_SIMPLEWALLET_INCOMING_TRANSACTION_H
#define IOTA_SIMPLEWALLET_INCOMING_TRANSACTION_H
#include <stdint.h>
#include <sqlite3.h>
#include <cjson/cJSON.h>
int create_incoming_transaction(sqlite3* db, const char* address, uint64_t amount, const char* bundle, const char* hash, const char* time, int confirmed);
int delete_account_incoming_transaction(sqlite3* db, const char* username);

cJSON* get_incoming_transaction_by_address(sqlite3* db, const char* address);

cJSON* get_incoming_transaction_hash(sqlite3* db, const char* hash);
cJSON* get_all_incoming_transactions(sqlite3* db, const char* username, uint32_t offset, uint32_t limit);
//Returns all inputs for a user account which we have not yet associated with an output spend
cJSON* get_unspents_by_username(sqlite3* db, const char* username);
int mark_incoming_transaction_confirmed(sqlite3* db, const char* hash);
#endif //IOTA_SIMPLEWALLET_INCOMING_TRANSACTION_H
