//
// Created by matth on 2/18/2020.
//

#ifndef IOTA_SIMPLEWALLET_ACCOUNT_H
#define IOTA_SIMPLEWALLET_ACCOUNT_H
#include <stdint.h>
#include <sqlite3.h>
#include <cjson/cJSON.h>
int create_account(sqlite3* db, const char* username, const char* seed_c, const char* salt, const char* nonce);
cJSON* get_account_by_username(sqlite3* db, const char* username);
#endif //IOTA_SIMPLEWALLET_ACCOUNT_H
