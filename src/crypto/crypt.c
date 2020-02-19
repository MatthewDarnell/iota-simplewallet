//
// Created by matth on 2/15/2020.
//

#include <string.h>
#include <sodium.h>
#include "../config/logger.h"
#include "crypt.h"

int init_crypto() {
   return sodium_init();
}

int encrypt(unsigned char* c, size_t* c_len, unsigned char* salt, size_t max_salt_len, size_t* salt_len, unsigned char* nonce, size_t max_nonce_len, size_t* nonce_len, char *data, size_t len, char *password) {
  if(max_salt_len < crypto_pwhash_SALTBYTES) {
    log_error("Salt isn't long enough, must be at least %d bytes.", crypto_pwhash_SALTBYTES);
    return -1;
  }

  if(max_nonce_len < crypto_secretbox_NONCEBYTES) {
    log_error("Nonce isn't long enough, must be at least %d bytes.", crypto_secretbox_NONCEBYTES);
    return -1;
  }
  const int key_len = crypto_box_SEEDBYTES;
  unsigned char key[key_len];

  *nonce_len = crypto_secretbox_NONCEBYTES;
  *salt_len = crypto_pwhash_SALTBYTES;
  randombytes_buf(salt, *salt_len);

  //Derive a key from our plaintext password and salt
  if (crypto_pwhash
        (key, key_len, password, strlen(password), salt,
         crypto_pwhash_OPSLIMIT_INTERACTIVE, crypto_pwhash_MEMLIMIT_INTERACTIVE,
         crypto_pwhash_ALG_DEFAULT) != 0) {

    log_fatal("Encrypting Data ran out of memory!", "");
  }
  *c_len = crypto_secretbox_MACBYTES + len;
  randombytes_buf(nonce, *nonce_len);
  //Encrypt the data
  crypto_secretbox_easy(c, (unsigned char*)data, len, nonce, key);
  return 0;

}

int decrypt(unsigned char *data, size_t max_data_len, unsigned char* c, size_t c_len, unsigned char* salt, unsigned char* nonce, char* password) {
  const int key_len = crypto_box_SEEDBYTES;
  unsigned char key[key_len];
  sodium_memzero(key, key_len);

  //Derive a key from our plaintext password and salt
  if (crypto_pwhash
        (key, key_len, password, strlen(password), salt,
         crypto_pwhash_OPSLIMIT_INTERACTIVE, crypto_pwhash_MEMLIMIT_INTERACTIVE,
         crypto_pwhash_ALG_DEFAULT) != 0) {

    log_fatal("Encrypting Data ran out of memory!", "");
  }

  sodium_memzero(data, max_data_len);
  if (crypto_secretbox_open_easy(data, c, c_len, nonce, key) != 0) {
    log_fatal("Data has been altered!", "")
    return -1;
  }
  return 0;
}