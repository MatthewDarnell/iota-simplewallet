//
// Created by matth on 2/18/2020.
//

#include <string.h>
#include <sodium.h>
#include "../../../crypto/password.h"
#include "../../../crypto/crypt.h"
#include "../../../iota/generate_seed.h"
#include "../../../config/logger.h"
#include "../stores/account.h"
#include "account.h"

int create_account(sqlite3* db, const char* username, char* password) {
  //Generate a secure password to store on disk from plaintext
  char hashed[128] = { 0 };
  hash_password(hashed, 128, password);
  int verified = verify_password(hashed, password); //Make sure it matches back up
  if(verified < 0) {
    log_error("Error Generating Account. (Password Mismatch!) %d", verified);
    return -1;
  }

  unsigned char c[128] = { 0 };
  unsigned char s[128] = { 0 };
  unsigned char n[128] = { 0 };

  size_t c_len = 0;
  size_t s_len = 0;
  size_t n_len = 0;



  char seed[128] = { 0 };
  generate_seed(seed, 128);

  //int encrypt(unsigned char* c, size_t* c_len, unsigned char* salt, size_t max_salt_len, unsigned char* nonce, size_t max_nonce_len, char *data, size_t len, char *password) {
  int encrypt_result = encrypt(
      c,
      &c_len,
      s,
      128,
      &s_len,
      n,
      128,
      &n_len,
      seed,
      81,
      password
    );
  sodium_memzero(seed, 128);  //Clear sensitive data from stack immediately
  destroy_password(password);

  if(encrypt_result < 0) {
    log_fatal("Failed to Create Account! %d", encrypt_result);
  }

  char b64_cipher[256] = { 0 };
  char b64_salt[256] = { 0 };
  char b64_nonce[256] = { 0 };

  sodium_bin2base64(b64_cipher, 256, c, c_len, sodium_base64_VARIANT_ORIGINAL_NO_PADDING);
  sodium_bin2base64(b64_salt, 256, s, s_len, sodium_base64_VARIANT_ORIGINAL_NO_PADDING);
  sodium_bin2base64(b64_nonce, 256, c, n_len, sodium_base64_VARIANT_ORIGINAL_NO_PADDING);

  return _create_account(
    db,
    username,
    b64_cipher,
    b64_salt,
    b64_nonce
    );
}
