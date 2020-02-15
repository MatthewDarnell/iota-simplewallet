//
// Created by matth on 2/15/2020.
//

#include <sodium.h>
#include "crypt.h"

int init_crypto() {
    return sodium_init();
}
char *generate_random(size_t len);
int encrypt(char *data, size_t len, char *key, size_t keylen);
int decrypt(char *data, size_t len, char *key, size_t keylen);