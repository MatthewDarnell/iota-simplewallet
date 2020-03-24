//
// Created by Matthew Darnell on 3/23/20.
//

#include <string.h>
#include "../../../iota-simplewallet.h"
#include "user_data.h"

#define enforce_max_length(len) if(len > 1024) return -1;
#define enforce_max_length_null(len) if(len > 1024) return NULL;
int _write_user_data(sqlite3* db, const char* username, const char* key, const char* value) {
  enforce_max_length(strlen(username) + strlen(key) + strlen(value))
  sqlite3_stmt* stmt;
  int rc;

  char* query = "INSERT INTO user_data(username, key, value) VALUES(?, ?, ?)";
  rc = sqlite3_prepare_v2(db, query, -1, &stmt, 0);

  if (rc != SQLITE_OK) {
    log_wallet_error("%s -- Failed to create prepared statement: %s", __func__, sqlite3_errmsg(db));
    return -1;
  }

  sqlite3_bind_text(stmt, 1, username, -1, NULL);
  sqlite3_bind_text(stmt, 2, key, -1, NULL);
  sqlite3_bind_text(stmt, 3, value, -1, NULL);

  while((rc = sqlite3_step(stmt)) == SQLITE_BUSY) {
    sqlite3_sleep(100);
  }

  if (rc != SQLITE_DONE) {
    log_wallet_error("%s execution failed: %s", __func__, sqlite3_errmsg(db));
    sqlite3_reset(stmt);
    sqlite3_finalize(stmt);
    return -1;
  }
  log_wallet_debug("User.(%s) updated key <%s>.", username, key);
  sqlite3_finalize(stmt);
  return 0;
}

int _delete_user_data(sqlite3* db, const char* username, const char* key) {
  enforce_max_length(strlen(username) + strlen(key))
  sqlite3_stmt* stmt;
  int rc;

  char* query = "DELETE FROM user_data WHERE username=? AND key=?";
  rc = sqlite3_prepare_v2(db, query, -1, &stmt, 0);

  if (rc != SQLITE_OK) {
    log_wallet_error("%s -- Failed to create prepared statement: %s", __func__, sqlite3_errmsg(db));
    return -1;
  }

  sqlite3_bind_text(stmt, 1, username, -1, NULL);
  sqlite3_bind_text(stmt, 2, key, -1, NULL);

  while((rc = sqlite3_step(stmt)) == SQLITE_BUSY) {
    sqlite3_sleep(100);
  }

  if (rc != SQLITE_DONE) {
    log_wallet_error("%s execution failed: %s", __func__, sqlite3_errmsg(db));
    sqlite3_reset(stmt);
    sqlite3_finalize(stmt);
    return -1;
  }
  log_wallet_debug("%s: User.(%s) Successfully Deleted Key <%s>.", __func__, username, key);
  sqlite3_finalize(stmt);
  return 0;
}

cJSON* _read_user_data(sqlite3* db, const char* username, const char* key) {
  enforce_max_length_null(strlen(username) + strlen(key))
  sqlite3_stmt* stmt;
  int rc;

  char* query = "SELECT * FROM user_data WHERE username=? AND key=?";
  rc = sqlite3_prepare_v2(db, query, -1, &stmt, 0);

  if (rc != SQLITE_OK) {
    log_wallet_error("%s -- Failed to create prepared statement: %s", __func__, sqlite3_errmsg(db));
    return NULL;
  }

  sqlite3_bind_text(stmt, 1, username, -1, NULL);
  sqlite3_bind_text(stmt, 2, key, -1, NULL);

  while((rc = sqlite3_step(stmt)) == SQLITE_BUSY) {
    sqlite3_sleep(100);
  }

  cJSON *json = NULL;
  if (rc == SQLITE_ROW) {
    json =  cJSON_CreateObject();

    cJSON_AddStringToObject(json, "username", (char*)sqlite3_column_text(stmt, 0 ));
    cJSON_AddStringToObject(json, "key", (char*)sqlite3_column_text(stmt, 1 ));
    cJSON_AddStringToObject(json, "value", (char*)sqlite3_column_text(stmt, 2 ));
    cJSON_AddStringToObject(json, "created_at", (char*)sqlite3_column_text(stmt, 3));
  }

  sqlite3_finalize(stmt);
  return json;
}
