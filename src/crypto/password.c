//
// Created by matth on 2/18/2020.
//

#include <stdio.h>
#include <string.h>
#include <sodium.h>
#include "../config/logger.h"
#include "password.h"

void hash_password(char* hashed, size_t max_hashed_len, char* password) {
  if (crypto_pwhash_str
        (hashed, password, strlen(password),
         crypto_pwhash_OPSLIMIT_SENSITIVE, crypto_pwhash_MEMLIMIT_SENSITIVE) != 0) {
    /* out of memory */
  }
}

int verify_password(char* hashed, char* password) {
  if (crypto_pwhash_str_verify
        (hashed, password, strlen(password)) != 0) {
    return -1;
  }
  return 0;
}