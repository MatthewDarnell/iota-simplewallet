//
// Created by Matthew Darnell on 3/23/20.
//

#ifndef IOTA_SIMPLEWALLET_USER_DATA_H
#define IOTA_SIMPLEWALLET_USER_DATA_H
#include <sqlite3.h>
#include <cjson/cJSON.h>
int _write_user_data(sqlite3* db, const char* username, const char* key, const char* value);
int _delete_user_data(sqlite3* db, const char* username, const char* key);
cJSON* _read_user_data(sqlite3* db, const char* username, const char* key);
#endif //IOTA_SIMPLEWALLET_USER_DATA_H
