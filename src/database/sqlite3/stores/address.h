//
// Created by matth on 2/18/2020.
//
#ifndef IOTA_SIMPLEWALLET_ADDRESS_H
#define IOTA_SIMPLEWALLET_ADDRESS_H
#include <stdint.h>
#include <sqlite3.h>
#include <cjson/cJSON.h>
int create_address(sqlite3* db, const char* address, uint32_t offset, const char* username);
cJSON* get_address_by_address(sqlite3* db, const char* address);
cJSON* get_next_fresh_address(sqlite3* db, const char* username);
cJSON* get_next_change_address(sqlite3* db, const char* username);
cJSON* get_deposit_addresses(sqlite3* db);
cJSON* get_addresses_for_spending(sqlite3* db, const char* username);
int32_t set_address_balance(sqlite3* db, const char* address, const char* balance);
int32_t get_num_change_addresses(sqlite3* db, const char* username);
int32_t get_num_fresh_addresses(sqlite3* db, const char* username);
int32_t get_latest_offset(sqlite3* db, const char* username);
int mark_address_spent_from(sqlite3* db, const char* address);
int mark_address_is_change_address(sqlite3* db, const char* address);
#endif //IOTA_SIMPLEWALLET_ADDRESS_H
