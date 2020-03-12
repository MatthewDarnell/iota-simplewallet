//
// Created by matth on 3/8/2020.
//

#include <stdlib.h>
#include <string.h>
#include <sodium.h>
#include <sqlite3.h>
#include "../crypto/crypt.h"
#include "../database/helpers/get_inputs.h"
#include "../database/sqlite3/stores/account.h"
#include "../database/helpers/generate_address.h"
#include "../database/sqlite3/db.h"
#include "../iota/api.h"
#include "../iota-simplewallet.h"

int __create_account(const char* username, char* password, const char* imported_seed) {
  //Generate a secure password to derive encryption key
  sqlite3* db = get_db_handle();

  log_wallet_info("Creating new account.(%s)", username);
  unsigned char c[128] = { 0 };
  unsigned char s[128] = { 0 };
  unsigned char n[128] = { 0 };

  size_t c_len = 0;
  size_t s_len = 0;
  size_t n_len = 0;

  char seed[128] = { 0 };
  if(imported_seed) {
    memcpy(seed, imported_seed, 81);
  } else {
    generate_seed(seed, 128);
  }

  int encrypt_result = encrypt_data(
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

  int ret_val = _create_account(
    db,
    username,
    b64_cipher,
    b64_salt,
    b64_nonce
  );
  close_db_handle(db);
  return ret_val;
}

char* get_accounts() {
  sqlite3* db = get_db_handle();
  cJSON* json = get_all_accounts(db);
  close_db_handle(db);
  if(!json) {
    return NULL;
  } else {
    char *ret_val = cJSON_Print(json);
    cJSON_Delete(json);
    return ret_val;  }
}

int verify_login(const char* username, char* password, int zero_password) {
  sqlite3* db = get_db_handle();
  cJSON* user = NULL;
  user = get_account_by_username(db, username);

  close_db_handle(db);
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

  int decrypt_result = decrypt_data(
    p,
    256,
    c,
    c_len,
    s,
    n,
    password
  );

  if(decrypt_result == 0) {
    get_account_inputs(username, (const char*)p); //Do we need to sync this account to find inputs
    generate_address(username, (const char*)p); //Seed is decrypted, let's see if we need to generate any more addresses
  }
  sodium_memzero(p, 128);

  if(zero_password != 0) {
    sodium_memzero(password, strlen(password));
  }




  if(decrypt_result < 0 ) {
    log_wallet_error("Invalid password, unable to login", "")
    return -1;
  }
  log_wallet_info("Logged in successfully", "")
  return 0;
}

int decrypt_seed(char* out, int out_max_len, const char* username, char* password) {
  sqlite3* db = get_db_handle();
  cJSON* user = get_account_by_username(db, username);
  close_db_handle(db);
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

  int decrypt_result = decrypt_data(
    p,
    256,
    c,
    c_len,
    s,
    n,
    password
  );


  if(decrypt_result < 0 ) {
    log_wallet_error("Invalid password, unable to decrypt seed", "")
    return -1;
  }

  memcpy(
    out,
    p,
    strlen((char*)p) > out_max_len ? out_max_len : strlen((char*)p)
  );

  sodium_memzero(p, 128);
  sodium_memzero(password, strlen(password));

  return 0;
}
