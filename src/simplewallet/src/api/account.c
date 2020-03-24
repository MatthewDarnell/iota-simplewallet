//
// Created by matth on 3/8/2020.
//

#include <string.h>
#include <sodium.h>
#include <sqlite3.h>
#include <cjson/cJSON.h>
#include <pthread.h>
#include "../crypto/crypt.h"
#include "../database/helpers/get_inputs.h"
#include "../database/sqlite3/stores/account.h"
#include "../database/sqlite3/stores/address.h"
#include "../database/sqlite3/stores/incoming_transaction.h"
#include "../database/sqlite3/stores/outgoing_transaction.h"
#include "../database/sqlite3/stores/user_data.h"
#include "../database/helpers/generate_address.h"
#include "../database/sqlite3/db.h"
#include "../iota/api.h"
#include "../iota-simplewallet.h"
pthread_mutex_t account_state_file_mutex = PTHREAD_MUTEX_INITIALIZER;
int __create_account(const char* username, char* password, const char* imported_seed) {
  if(!username || !password) {
    log_wallet_error("%s invalid parameters", __func__);
    return -1;
  }
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
    close_db_handle(db);
    return -1;
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

int _verify_login(const char* username, char* password, int zero_password, int generate_inputs) {
  if(!username || !password) {
    log_wallet_error("%s invalid parameters", __func__);
    return -1;
  }
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

  if(decrypt_result == 0 && generate_inputs > 0) {
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

int delete_account(const char* username, char* password) {
  if(_verify_login(username, password, 0, 0) < 0) {
    log_wallet_error("%s Invalid Login Credentials", __func__);
    return -1;
  }
  sqlite3* db = get_db_handle();
  if(0 == delete_account_outgoing_transaction(db, username)) {
    if(0 == delete_account_incoming_transaction(db, username)) {
      if(0 == delete_account_addresses(db, username)) {
        if(0 == _delete_account(db, username)) {
          close_db_handle(db);
          return 0;
        }
      }
    }
  }
  close_db_handle(db);
  return -1;
};

char* get_accounts() {
  sqlite3* db = get_db_handle();
  cJSON* json = get_all_accounts(db);
  close_db_handle(db);
  if(!json) {
    return NULL;
  } else {
    char *ret_val = cJSON_Print(json);
    cJSON_Delete(json);
    return ret_val;
  }
}


int decrypt_seed(char* out, int out_max_len, const char* username, char* password) {
  if(!username || !password) {
    log_wallet_error("%s invalid parameters", __func__);
    return -1;
  }
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

int export_account_state(const char* username, char* password, const char* path) {
  if(verify_login(username, password, 0) < 0) {
    log_wallet_error("%s Invalid Login Credentials", __func__);
    return -1;
  }
  sqlite3* db = get_db_handle();
  cJSON* account = get_account_by_username(db, username);
  if(!account) {
    log_wallet_error("%s Could not get Account. Does your username exist?", __func__);
    close_db_handle(db);
    return -1;
  }

  int synced = cJSON_GetObjectItem(account, "is_synced")->valueint;
  if(synced == 0) {
    cJSON_Delete(account);
    log_wallet_error("%s account <%s> is not synced. Cannot export account state.", __func__, username);
    close_db_handle(db);
    return -1;
  }

  cJSON* temp_address, *addresses = get_all_addresses_by_username(db, username);
  cJSON* address_list = cJSON_CreateArray();
  cJSON_ArrayForEach(temp_address, addresses) {
    int offset = cJSON_GetObjectItem(temp_address, "offset")->valueint;
    char* balance = cJSON_GetObjectItem(temp_address, "balance")->valuestring;
    int spent_from = cJSON_GetObjectItem(temp_address, "spent_from")->valueint;
    int used = cJSON_GetObjectItem(temp_address, "used")->valueint;

    uint64_t d_balance = strtoull(balance, NULL, 10);

    if(spent_from || used || d_balance > 0) {
      cJSON* obj = cJSON_CreateObject();
      cJSON_AddNumberToObject(obj, "offset", offset);
      cJSON_AddNumberToObject(obj, "spent_from", spent_from);
      cJSON_AddNumberToObject(obj, "used", used);
      cJSON_AddStringToObject(obj, "balance", balance);
      cJSON_AddItemToArray(address_list, obj);
    }
  }

  close_db_handle(db);

  cJSON_AddItemToObject(account, "addresses", address_list);
  char* state = cJSON_Print(account);
  cJSON_Delete(account);


  pthread_mutex_lock(&account_state_file_mutex);
  FILE* i_file = fopen(path, "wb+");
  if(!i_file) {
    free(state);
    pthread_mutex_unlock(&account_state_file_mutex);
    log_wallet_error("%s unable to open file for writing account %s", __func__, path);
    return -1;
  }
  fprintf(i_file, "%s", state);
  fclose(i_file);
  pthread_mutex_unlock(&account_state_file_mutex);
  free(state);
  log_wallet_info("%s writing file <%s> was successful", __func__, path);
  return 0;
}

int import_account_state(char* password, const char* path) {
  if(!path) {
    log_wallet_error("%s No path provided", __func__);
    return -1;
  }

  pthread_mutex_lock(&account_state_file_mutex);

  FILE* i_file = fopen(path, "rb+");
  if(!i_file) {
    pthread_mutex_unlock(&account_state_file_mutex);
    log_wallet_error("%s Could not open file %s", __func__, path);
    return -1;
  }

  fseek(i_file, 0, SEEK_END);
  long f_size = ftell(i_file);
  rewind(i_file);
  char* buffer = calloc(f_size + 1, sizeof(char));

  if(!buffer) {
    fclose(i_file);
    pthread_mutex_unlock(&account_state_file_mutex);
    log_wallet_error("%s OOM", __func__);
    return -1;
  }
  if(f_size != fread(buffer, sizeof(char), f_size, i_file)) {
    fclose(i_file);
    pthread_mutex_unlock(&account_state_file_mutex);
    free(buffer);
    log_wallet_error("%s Could not read file", __func__);
    return -1;
  }
  fclose(i_file);
  pthread_mutex_unlock(&account_state_file_mutex);

  cJSON* json = cJSON_Parse(buffer);
  free(buffer);

  if(!json) {
    log_wallet_error("%s could not read file. (Was it altered?)", __func__);
    return -1;
  }


  //Validate json
  if(!cJSON_IsObject(json)) {
    cJSON_Delete(json);
    log_wallet_error("%s could not read file. (Was it altered?)", __func__);
    return -1;
  }

  if(
    !cJSON_HasObjectItem(json, "username") ||
    !cJSON_HasObjectItem(json, "seed_ciphertext") ||
    !cJSON_HasObjectItem(json, "salt") ||
    !cJSON_HasObjectItem(json, "nonce") ||
    !cJSON_HasObjectItem(json, "balance") ||
    !cJSON_HasObjectItem(json, "is_synced") ||
    !cJSON_HasObjectItem(json, "addresses")
    ) {
    cJSON_Delete(json);
    log_wallet_error("%s could not read file. (Was it altered?)", __func__);
    return -1;
  }

  cJSON* addresses = cJSON_GetObjectItem(json, "addresses");

  if(!cJSON_IsArray(addresses)) {
    cJSON_Delete(json);
    log_wallet_error("%s could not read file. (Was it altered?)", __func__);
    return -1;
  }

  int num_addresses = cJSON_GetArraySize(addresses);
  cJSON* address = NULL;

  cJSON_ArrayForEach(address, addresses) {
    if(
      !cJSON_HasObjectItem(address, "offset") ||
      !cJSON_HasObjectItem(address, "spent_from") ||
      !cJSON_HasObjectItem(address, "used") ||
      !cJSON_HasObjectItem(address, "balance")
    ) {
      cJSON_Delete(json);
      log_wallet_error("%s could not read file. (Was it altered?)", __func__);
      return -1;
    }

    if(
      !cJSON_IsNumber(cJSON_GetObjectItem(address, "offset")) ||
      !cJSON_IsNumber(cJSON_GetObjectItem(address, "spent_from")) ||
      !cJSON_IsNumber(cJSON_GetObjectItem(address, "used")) ||
      !cJSON_IsString(cJSON_GetObjectItem(address, "balance"))
      ) {
      cJSON_Delete(json);
      log_wallet_error("%s could not read file. (Was it altered?)", __func__);
      return -1;
    }
  }

  //Validated
  sqlite3* db = get_db_handle();

  char* username = cJSON_GetObjectItem(json, "username")->valuestring;
  char* seed_cipher = cJSON_GetObjectItem(json, "seed_ciphertext")->valuestring;
  char* nonce = cJSON_GetObjectItem(json, "nonce")->valuestring;
  char* salt = cJSON_GetObjectItem(json, "salt")->valuestring;

  //Does user already exist?
  cJSON* existing_user = get_account_by_username(db, username);
  if(existing_user) {
    log_wallet_error("%s: Importing user (%s) which already exists\n", __func__, username);
    cJSON_Delete(existing_user);
    close_db_handle(db);
    cJSON_Delete(json);
    return -1;
  }



  if(0 != _create_account(
    db,
    username,
    seed_cipher,
    salt,
    nonce
  )) {
    close_db_handle(db);
    cJSON_Delete(json);
    log_wallet_error("%s Could not create account, db error", __func__);
    return -1;
  }

  uint32_t last_offset = 0;
  int i;

  for(i = 0; i < num_addresses; i++) {
    address = cJSON_GetArrayItem(addresses, i);
    int offset = cJSON_GetObjectItem(address, "offset")->valueint;
    if(offset > last_offset) {
      last_offset = offset;
    }
  }

  char seed[128] = { 0 };
  if(0 != decrypt_seed(seed, 127, username, password)) {
    cJSON_Delete(json);
    close_db_handle(db);
    log_wallet_error("%s Invalid Password", __func__);
    return -1;
  }
  //Need to generate at least <last_offset> addresses
  cJSON* generated_addresses = generate_new_addresses(seed, 0, last_offset + 1);

  cJSON* address_to_check = NULL;
  cJSON_ArrayForEach(address, generated_addresses) {
    const char* temp_address = cJSON_GetObjectItem(address, "address")->valuestring;
    int temp_index = cJSON_GetObjectItem(address, "index")->valueint;

    int spent_from = 0,
        used = 0;
    uint64_t d_balance = 0;

    cJSON_ArrayForEach(address_to_check, addresses) {
      int offset = cJSON_GetObjectItem(address_to_check, "offset")->valueint;
      if(temp_index == offset) {
        spent_from = cJSON_GetObjectItem(address_to_check, "spent_from")->valueint;
        used = cJSON_GetObjectItem(address_to_check, "used")->valueint;
        d_balance = strtoull(cJSON_GetObjectItem(address_to_check, "balance")->valuestring, NULL, 10);
        break;
      }
    }

    if(0 != create_address(db, temp_address, temp_index, username)) {
      log_wallet_error("%s error creating address <%s> when importing (offset.(%d) )", __func__, temp_address, temp_index);
    } else {
      if(spent_from > 0) {
        mark_address_spent_from(db, temp_address);
      }
      if(used > 0) {
        mark_address_used(db, temp_address);
      }
      if(d_balance > 0) {
        char b[128] = { 0 };
#ifdef WIN32
        snprintf(b, 127, "%I64u", d_balance);
#else
    snprintf(b, 127, "%llu", d_balance);
#endif
        set_address_balance(db, temp_address, b);
      }
    }
  }


  close_db_handle(db);
  log_wallet_info("%s User <%s> imported successfully", __func__, username);
  cJSON_Delete(json);
  return 0;
}



int write_user_data(const char* username, const char* key, const char* value) {
  if(!username || !key || !value) {
    log_wallet_error("%s invalid parameters", __func__);
    return -1;
  }
  //Generate a secure password to derive encryption key
  sqlite3* db = get_db_handle();
  int ret_val = _write_user_data(db, username, key, value);
  close_db_handle(db);
  return ret_val;
}

int delete_user_data(const char* username, const char* key) {
  if(!username || !key) {
    log_wallet_error("%s invalid parameters", __func__);
    return -1;
  }
  //Generate a secure password to derive encryption key
  sqlite3* db = get_db_handle();
  int ret_val = _delete_user_data(db, username, key);
  close_db_handle(db);
  return ret_val;
}

char* read_user_data(const char* username, const char* key) {
  if(!username || !key) {
    log_wallet_error("%s invalid parameters", __func__);
    return NULL;
  }
  //Generate a secure password to derive encryption key
  sqlite3* db = get_db_handle();
  cJSON* json = _read_user_data(db, username, key);
  close_db_handle(db);
  if(!json) {
    return NULL;
  }
  char* ret_val = cJSON_PrintUnformatted(json);
  cJSON_Delete(json);
  return ret_val;
}