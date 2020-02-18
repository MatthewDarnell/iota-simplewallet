//
// Created by matth on 2/18/2020.
//

#ifndef IOTA_SIMPLEWALLET_INPUT_TO_OUTPUT_H
#define IOTA_SIMPLEWALLET_INPUT_TO_OUTPUT_H
#include <stdint.h>
#include <sqlite3.h>
#include <cjson/cJSON.h>
int create_input_to_output(sqlite3* db, const char* input_hash, int64_t output_serial);
cJSON* get_inputs_for_output(sqlite3* db, int64_t output_serial);
#endif //IOTA_SIMPLEWALLET_INPUT_TO_OUTPUT_H
