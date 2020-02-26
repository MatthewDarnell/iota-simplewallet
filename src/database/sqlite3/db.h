//
// Created by matth on 2/16/2020.
//

#ifndef IOTA_SIMPLEWALLET_DB_H
#define IOTA_SIMPLEWALLET_DB_H
#include <sqlite3.h>
int init_db();
sqlite3* get_db_handle();
void close_db_handle(sqlite3* handle);
#endif //IOTA_SIMPLEWALLET_DB_H
