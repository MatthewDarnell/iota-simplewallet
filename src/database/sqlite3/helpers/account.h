//
// Created by matth on 2/18/2020.
//

#ifndef IOTA_SIMPLEWALLET_ACCOUNT_H
#define IOTA_SIMPLEWALLET_ACCOUNT_H
#include <sqlite3.h>
int __create_account(sqlite3* db, const char* username, char* password, const char* imported_seed);

#define create_account(db, username, password) __create_account(db, username, password, NULL)
#define import_account(db, username, password, seed) __create_account(db, username, password, seed)


char* get_accounts(sqlite3* db);
int verify_login(sqlite3* db, const char* username, char* password);
#endif //IOTA_SIMPLEWALLET_ACCOUNT_H
