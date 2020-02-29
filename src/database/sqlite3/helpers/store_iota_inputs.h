//
// Created by matth on 2/26/2020.
//

#ifndef IOTA_SIMPLEWALLET_STORE_IOTA_INPUTS_H
#define IOTA_SIMPLEWALLET_STORE_IOTA_INPUTS_H
#include <sqlite3.h>
int store_inputs(sqlite3* db, char* str_inputs);
#endif //IOTA_SIMPLEWALLET_STORE_IOTA_INPUTS_H
