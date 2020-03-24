//
// Created by matth on 2/18/2020.
//

#include <string.h>
#include <sodium.h>
#include "random.h"
void generate_random(char* out, size_t len) {
  sodium_memzero(out, len);
  randombytes_buf(out, len);
}
