//
// Created by matth on 2/15/2020.
//

#ifndef IOTA_SIMPLEWALLET_CRYPT_H
#define IOTA_SIMPLEWALLET_CRYPT_H
#include <stddef.h>
int init_crypto();
int encrypt(unsigned char* c, size_t* c_len, unsigned char* salt, size_t max_salt_len, size_t* salt_len, unsigned char* nonce, size_t max_nonce_len, size_t* nonce_len, char *data, size_t len, char *password);
int decrypt(unsigned char *data, size_t max_data_len, unsigned char* c, size_t c_len, unsigned char* salt, unsigned char* nonce, char* password);
#endif //IOTA_SIMPLEWALLET_CRYPT_H
