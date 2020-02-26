//
// Created by matth on 2/18/2020.
//

#include <string.h>
#include <sodium.h>
#include "../../../crypto/password.h"
#include "../../../crypto/crypt.h"
#include "../../../iota/api.h"
#include "../../../config/logger.h"
#include "../stores/account.h"
#include "account.h"

int create_account(sqlite3* db, const char* username, char* password) {
  //Generate a secure password to derive encryption key

  unsigned char c[128] = { 0 };
  unsigned char s[128] = { 0 };
  unsigned char n[128] = { 0 };

  size_t c_len = 0;
  size_t s_len = 0;
  size_t n_len = 0;

  char seed[128] = { 0 };
  generate_seed(seed, 128);

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
  sodium_memzero(password, strlen(password));

  if(encrypt_result < 0) {
    log_wallet_fatal("Failed to Create Account! %d", encrypt_result);
  }

  char b64_cipher[256] = { 0 };
  char b64_salt[256] = { 0 };
  char b64_nonce[256] = { 0 };

  sodium_bin2base64(b64_cipher, 256, c, c_len, sodium_base64_VARIANT_ORIGINAL_NO_PADDING);
  sodium_bin2base64(b64_salt, 256, s, s_len, sodium_base64_VARIANT_ORIGINAL_NO_PADDING);
  sodium_bin2base64(b64_nonce, 256, n, n_len, sodium_base64_VARIANT_ORIGINAL_NO_PADDING);

  return _create_account(
    db,
    username,
    b64_cipher,
    b64_salt,
    b64_nonce
    );
}
int verify_login(sqlite3* db, const char* username, char* password) {
  cJSON* user = get_account_by_username(db, username);
  if(!user) {
    log_wallet_error("User %s does not exist", username);
    return -1;
  }

  char* b64_cipher = cJSON_GetObjectItem(user, "seed_ciphertext")->valuestring;
  char* b64_nonce = cJSON_GetObjectItem(user, "nonce")->valuestring;
  char* b64_salt = cJSON_GetObjectItem(user, "salt")->valuestring;

  unsigned char c[128] = { 0 };
  unsigned char s[128] = { 0 };
  unsigned char n[128] = { 0 };

  size_t c_len = 0,
         s_len = 0,
         n_len = 0;

  sodium_base642bin(c, 128, b64_cipher, strlen(b64_cipher), NULL, &c_len, NULL, sodium_base64_VARIANT_ORIGINAL_NO_PADDING);
  sodium_base642bin(n, 128, b64_nonce, strlen(b64_nonce), NULL, &n_len, NULL, sodium_base64_VARIANT_ORIGINAL_NO_PADDING);
  sodium_base642bin(s, 128, b64_salt, strlen(b64_salt), NULL, &s_len, NULL, sodium_base64_VARIANT_ORIGINAL_NO_PADDING);
  cJSON_Delete(user);

  unsigned char p[256] = { 0 };

  int decrypt_result = decrypt(
    p,
    256,
    c,
    c_len,
    s,
    n,
    password
  );
  sodium_memzero(p, 128);
  sodium_memzero(password, strlen(password));


  if(decrypt_result < 0 ) {
    log_wallet_error("Invalid password, unable to login", "")
    return -1;
  }
  log_wallet_info("Logged in successfully", "")
  return 0;
}
