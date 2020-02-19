//
// Created by matth on 2/18/2020.
//

#ifndef IOTA_SIMPLEWALLET_GENERATE_SEED_H
#define IOTA_SIMPLEWALLET_GENERATE_SEED_H
#include <stdint.h>
void generate_seed(char* buffer, uint32_t buf_max_len);
void destroy_seed(char* buffer);
#endif //IOTA_SIMPLEWALLET_GENERATE_SEED_H
