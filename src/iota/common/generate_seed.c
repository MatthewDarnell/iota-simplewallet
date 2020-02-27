//
// Created by matth on 2/18/2020.
//

#include <string.h>
#include <sodium.h>
#include "../../crypto/random.h"

void generate_seed(char* buffer, uint32_t buf_max_len) {
  const char* char_set = "9ABCDEFGHIJKLMNOPQRSTUVWXYZ";
  sodium_memzero(buffer, buf_max_len);
  char random[256] = { 0 };
  int chars_found = 0;

  while(chars_found < 81) {
    sodium_stackzero(256);
    generate_random(random, 256);
    char* pch = strpbrk (random, char_set);
    while (pch != NULL) {
      buffer[chars_found] = *pch;
      chars_found++;
      if(chars_found >= 81) {
        break;
      }
      pch = strpbrk (pch + 1, char_set);
    }
  }
}

void destroy_seed(char* buffer) {
  sodium_memzero(buffer, 81);
}