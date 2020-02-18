//
// Created by matth on 2/18/2020.
//
#ifndef IOTA_SIMPLEWALLET_INCOMING_TRANSACTION_H
#define IOTA_SIMPLEWALLET_INCOMING_TRANSACTION_H
#include <stdint.h>
#include <sqlite3.h>
#include <cjson/cJSON.h>
int create_incoming_transaction(sqlite3* db, const char* address, uint64_t amount, const char* bundle, const char* hash, const char* time, int confirmed);
cJSON* get_incoming_transaction_by_address(sqlite3* db, const char* address);

//Returns all inputs for a user account which we have not yet associated with an output spend
cJSON* get_unspents_by_username(sqlite3* db, const char* username);
#endif //IOTA_SIMPLEWALLET_INCOMING_TRANSACTION_H
