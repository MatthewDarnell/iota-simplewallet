//
// Created by matth on 2/25/2020.
//

#ifndef IOTA_SIMPLEWALLET_API_H
#define IOTA_SIMPLEWALLET_API_H
#include <cjson/cJSON.h>
#include <cclient/service.h>
#include <stdint.h>
void init_iota_client();
void shutdown_iota_client();
iota_client_service_t* get_iota_client();
void free_iota_client(iota_client_service_t** serv);


cJSON* generate_new_addresses(const char* seed, int index, int num_addresses);
void generate_seed(char* buffer, uint32_t buf_max_len);
void get_address_balance(cJSON** addresses, uint64_t min_iota);
void get_latest_inclusion(cJSON** addresses_with_transactions, int include_unconfirmed);
void get_transaction_inputs_to_address(cJSON** addresses);
void init_iota();
void shutdown_iota();
#endif //IOTA_SIMPLEWALLET_API_H
