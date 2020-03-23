//
// Created by matth on 2/18/2020.
//

#ifndef IOTA_SIMPLEWALLET_PASSWORD_H
#define IOTA_SIMPLEWALLET_PASSWORD_H
void hash_password(char* hashed, size_t max_hashed_len, char* password);
//Returns 0 if valid, < 1 otherwise
int verify_password(char* hashed, char* password);
#endif //IOTA_SIMPLEWALLET_PASSWORD_H
