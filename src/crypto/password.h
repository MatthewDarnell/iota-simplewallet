//
// Created by matth on 2/18/2020.
//

#ifndef IOTA_SIMPLEWALLET_PASSWORD_H
#define IOTA_SIMPLEWALLET_PASSWORD_H
void hash_password(char* hashed, size_t max_hashed_len, char* password);
void get_key_and_salt_from_hashed_password(unsigned char* key, size_t max_key_len, unsigned char* salt, size_t max_salt_len, char* hashed_password);
//Returns 0 if valid, < 1 otherwise
int verify_password(char* hashed, char* password);
void destroy_password(char* password);
#endif //IOTA_SIMPLEWALLET_PASSWORD_H
