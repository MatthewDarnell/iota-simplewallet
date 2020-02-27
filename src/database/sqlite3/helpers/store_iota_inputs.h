//
// Created by matth on 2/26/2020.
//

#ifndef IOTA_SIMPLEWALLET_STORE_IOTA_INPUTS_H
#define IOTA_SIMPLEWALLET_STORE_IOTA_INPUTS_H
#include <sqlite3.h>
#include <cjson/cJSON.h>
int store_inputs(sqlite3* db, cJSON* inputs);
#endif //IOTA_SIMPLEWALLET_STORE_IOTA_INPUTS_H
