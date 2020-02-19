//
// Created by matth on 2/18/2020.
//

#ifndef IOTA_SIMPLEWALLET_ACCOUNT_H
#define IOTA_SIMPLEWALLET_ACCOUNT_H
#include <sqlite3.h>
int create_account(sqlite3* db, const char* username, char* password);
#endif //IOTA_SIMPLEWALLET_ACCOUNT_H
