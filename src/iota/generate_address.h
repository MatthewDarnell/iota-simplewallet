//
// Created by matth on 2/19/2020.
//

#ifndef IOTA_SIMPLEWALLET_GENERATE_ADDRESS_H
#define IOTA_SIMPLEWALLET_GENERATE_ADDRESS_H
#include <cjson/cJSON.h>
cJSON* get_new_address(const char* seed, int index, int num_addresses);
#endif //IOTA_SIMPLEWALLET_GENERATE_ADDRESS_H
