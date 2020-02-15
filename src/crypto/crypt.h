//
// Created by matth on 2/15/2020.
//

#ifndef IOTA_SIMPLEWALLET_CRYPT_H
#define IOTA_SIMPLEWALLET_CRYPT_H

int init_crypto();
char *generate_random(size_t len);
int encrypt(char *data, size_t len, char *key, size_t keylen);
int decrypt(char *data, size_t len, char *key, size_t keylen);

#endif //IOTA_SIMPLEWALLET_CRYPT_H
