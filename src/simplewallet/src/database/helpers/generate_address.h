//
// Created by matth on 2/27/2020.
//

#ifndef IOTA_SIMPLEWALLET_GENERATE_ADDRESS_H
#define IOTA_SIMPLEWALLET_GENERATE_ADDRESS_H
int generate_address(const char* username, const char* seed);
int _generate_num_addresses(const char* username, char* password, int num_addrs);
#endif //IOTA_SIMPLEWALLET_GENERATE_ADDRESS_H
