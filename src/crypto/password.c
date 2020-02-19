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

void get_key_and_salt_from_hashed_password(unsigned char* key, size_t max_key_len, unsigned char* salt, size_t max_salt_len, char* hashed_password) {
  if(max_key_len < crypto_box_SEEDBYTES || max_salt_len < crypto_pwhash_SALTBYTES) {
    log_fatal("Key (%d) Or Salt (%d) lengths too small!", max_key_len, max_salt_len);
  }


  randombytes_buf(salt, crypto_pwhash_SALTBYTES);
  if (crypto_pwhash
        (key, crypto_box_SEEDBYTES, hashed_password, strlen(hashed_password), salt,
         crypto_pwhash_OPSLIMIT_INTERACTIVE, crypto_pwhash_MEMLIMIT_INTERACTIVE,
         crypto_pwhash_ALG_DEFAULT) != 0) {

    log_fatal("Encrypting Data ran out of memory!", "");
  }
}


int verify_password(char* hashed, char* password) {
  if (crypto_pwhash_str_verify
        (hashed, password, strlen(password)) != 0) {
    return -1;
  }
  return 0;
}

void destroy_password(char* password) {
  sodium_memzero(password, strlen(password));
}