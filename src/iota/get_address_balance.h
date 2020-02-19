//
// Created by matth on 2/19/2020.
//

#ifndef IOTA_SIMPLEWALLET_GET_ADDRESS_BALANCE_H
#define IOTA_SIMPLEWALLET_GET_ADDRESS_BALANCE_H
#include <cjson/cJSON.h>
//@addresses is a cJSON  array of objects, i.e. [{address: xxx}, ...]
cJSON* get_address_balance(cJSON* addresses);
#endif //IOTA_SIMPLEWALLET_GET_ADDRESS_BALANCE_H
