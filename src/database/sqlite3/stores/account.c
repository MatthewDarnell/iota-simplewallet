//
// Created by matth on 2/18/2020.
//
#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include "../../../config/logger.h"
#include "account.h"

#define enforce_max_length(len) if(len > 1024) return -1;
#define enforce_max_length_null(len) if(len > 1024) return NULL;
int _create_account(sqlite3* db, const char* username, const char* seed_c, const char* salt, const char* nonce) {
  enforce_max_length(strlen(username) + strlen(seed_c) + strlen(salt) + strlen(nonce))
  sqlite3_stmt* stmt;
  int rc;

  char* query = "INSERT INTO account(username, seed_ciphertext, salt, nonce) VALUES(?, ?, ?, ?)";
  rc = sqlite3_prepare_v2(db, query, -1, &stmt, 0);

  if (rc != SQLITE_OK) {
    log_wallet_error("%s -- Failed to create prepared statement: %s", __func__, sqlite3_errmsg(db));
    return -1;
  }

  sqlite3_bind_text(stmt, 1, username, -1, NULL);
  sqlite3_bind_text(stmt, 2, seed_c, -1, NULL);
  sqlite3_bind_text(stmt, 3, salt, -1, NULL);
  sqlite3_bind_text(stmt, 4, nonce, -1, NULL);

  rc = sqlite3_step(stmt);

  if (rc != SQLITE_DONE) {
    log_wallet_error("%s execution failed: %s", __func__, sqlite3_errmsg(db));
    sqlite3_reset(stmt);
    sqlite3_finalize(stmt);
    return -1;
  }
  log_wallet_info("User.(%s) created successfully.", username);
  sqlite3_finalize(stmt);
  return 0;
}

cJSON* get_all_accounts(sqlite3* db) {
  sqlite3_stmt* stmt;
  int rc;

  char* query = "SELECT * FROM account ORDER BY created_at ASC";
  rc = sqlite3_prepare_v2(db, query, -1, &stmt, 0);

  if (rc != SQLITE_OK) {
    log_wallet_error("%s -- Failed to create prepared statement: %s", __func__, sqlite3_errmsg(db));
    return NULL;
  }

  rc = sqlite3_step(stmt);

  cJSON *json = cJSON_CreateArray();

  while(rc == SQLITE_ROW) {
    cJSON *row =  cJSON_CreateObject();
    cJSON_AddNumberToObject(row, "index", sqlite3_column_int(stmt, 0 ));
    cJSON_AddStringToObject(row, "username", (char*)sqlite3_column_text(stmt, 4 ));

    uint64_t balance = sqlite3_column_int64(stmt, 5);
    char str_balance[64] = { 0 };

#ifdef WIN32
    snprintf(str_balance, 64, "%I64d", balance);
#else
    snprintf(str_balance, 64, "%lld", balance);
#endif

    cJSON_AddStringToObject(row, "balance", str_balance);
    cJSON_AddNumberToObject(row, "is_synced", sqlite3_column_int(stmt, 6));
    cJSON_AddStringToObject(row, "created_at", (char*)sqlite3_column_text(stmt, 7));
    cJSON_AddItemToArray(json, row);
    rc = sqlite3_step(stmt);
  }

  sqlite3_finalize(stmt);
  return json;
}
int is_account_synced(sqlite3* db, const char* username) {
  enforce_max_length(strlen(username))
  sqlite3_stmt* stmt;
  int rc;

  char* query = "SELECT is_synced FROM account WHERE username=? LIMIT 1";
  rc = sqlite3_prepare_v2(db, query, -1, &stmt, 0);

  if (rc != SQLITE_OK) {
    log_wallet_error("%s -- Failed to create prepared statement: %s", __func__, sqlite3_errmsg(db));
    return -1;
  }

  sqlite3_bind_text(stmt, 1, username, -1, NULL);
  rc = sqlite3_step(stmt);
  int is_synced = 0;
  if (rc == SQLITE_ROW) {
    is_synced = sqlite3_column_int(stmt, 0 );
  }
  sqlite3_finalize(stmt);
  return is_synced;
}
cJSON* get_account_by_username(sqlite3* db, const char* username) {
  enforce_max_length_null(strlen(username))
  sqlite3_stmt* stmt;
  int rc;

  char* query = "SELECT * FROM account WHERE username=? LIMIT 1";
  rc = sqlite3_prepare_v2(db, query, -1, &stmt, 0);

  if (rc != SQLITE_OK) {
    log_wallet_error("%s -- Failed to create prepared statement: %s", __func__, sqlite3_errmsg(db));
    return NULL;
  }

  sqlite3_bind_text(stmt, 1, username, -1, NULL);
  rc = sqlite3_step(stmt);

  cJSON *json = NULL;
  if (rc == SQLITE_ROW) {
    json =  cJSON_CreateObject();

    cJSON_AddNumberToObject(json, "serial", sqlite3_column_int(stmt, 0 ));
    cJSON_AddStringToObject(json, "seed_ciphertext", (char*)sqlite3_column_text(stmt, 1 ));
    cJSON_AddStringToObject(json, "salt", (char*)sqlite3_column_text(stmt, 2 ));
    cJSON_AddStringToObject(json, "nonce", (char*)sqlite3_column_text(stmt, 3 ));
    cJSON_AddStringToObject(json, "username", (char*)sqlite3_column_text(stmt, 4 ));

    uint64_t balance = sqlite3_column_int64(stmt, 5);
    char str_balance[64] = { 0 };

    #ifdef WIN32
    snprintf(str_balance, 64, "%I64d", balance);
    #else
    snprintf(str_balance, 64, "%lld", balance);
    #endif

    cJSON_AddStringToObject(json, "balance", str_balance);
    cJSON_AddNumberToObject(json, "is_synced", sqlite3_column_int(stmt, 6));
    cJSON_AddStringToObject(json, "created_at", (char*)sqlite3_column_text(stmt, 7));
  }

  sqlite3_finalize(stmt);
  return json;
}
int mark_account_synced(sqlite3* db, const char* username) {
  enforce_max_length(  strlen(username))
  sqlite3_stmt* stmt;
  int rc;

  char* query = "UPDATE account SET is_synced=1  WHERE username=?";
  rc = sqlite3_prepare_v2(db, query, -1, &stmt, 0);

  if (rc != SQLITE_OK) {
    log_wallet_error( "%s -- Failed to create prepared statement: %s", __func__, sqlite3_errmsg(db));
    return -1;
  }

  sqlite3_bind_text(stmt, 1, username, -1, NULL);
  rc = sqlite3_step(stmt);

  if (rc != SQLITE_DONE) {
    log_wallet_error("%s execution failed: %s", __func__, sqlite3_errmsg(db));
    sqlite3_reset(stmt);
    sqlite3_finalize(stmt);
    return -1;
  } else {
    log_wallet_debug("Marked Account (<%s>) as Synced.\n", username);
  }
  sqlite3_finalize(stmt);
  return 0;
}