//
// Created by matth on 2/18/2020.
//

#ifndef IOTA_SIMPLEWALLET_ACCOUNT_H
#define IOTA_SIMPLEWALLET_ACCOUNT_H
#include <sqlite3.h>
int __create_account(const char* username, char* password, const char* imported_seed);

#define create_account(username, password) __create_account(username, password, NULL)
#define import_account(username, password, seed) __create_account(username, password, seed)


char* get_accounts();
int verify_login(const char* username, char* password);
#endif //IOTA_SIMPLEWALLET_ACCOUNT_H
