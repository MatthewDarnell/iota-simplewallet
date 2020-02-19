//
// Created by matth on 2/18/2020.
//

#include <stdio.h>
#include <string.h>
#include "../../../config/logger.h"
#include "address.h"

#define enforce_max_length(len) if(len > 1024) return -1;
#define enforce_max_length_null(len) if(len > 1024) return NULL;

int create_address(sqlite3* db, const char* address, uint32_t offset, const char* username) {
  enforce_max_length(strlen(address) + strlen(username) + 10)
  sqlite3_stmt* stmt;
  int rc;

  char* query = "INSERT INTO address(address, offset, account) VALUES(?, ?, ?)";
  rc = sqlite3_prepare_v2(db, query, -1, &stmt, 0);

  if (rc != SQLITE_OK) {
    log_error("%s -- Failed to create prepared statement: %s", __func__, sqlite3_errmsg(db));
    return -1;
  }

  sqlite3_bind_text(stmt, 1, address, -1, NULL);
  sqlite3_bind_int(stmt, 2, offset);
  sqlite3_bind_text(stmt, 3, username, -1, NULL);

  rc = sqlite3_step(stmt);

  if (rc != SQLITE_DONE) {
    log_error("%s execution failed: %s", __func__, sqlite3_errmsg(db));
    sqlite3_reset(stmt);
    sqlite3_finalize(stmt);
    return -1;
  }
  sqlite3_finalize(stmt);
  log_debug("Created new address %s for user %s", address, username);
  return 0;
}

cJSON* get_address_by_address(sqlite3* db, const char* address) {
  enforce_max_length_null(strlen(address))
  sqlite3_stmt* stmt;
  int rc;

  char* query = "SELECT * FROM address WHERE address=? LIMIT 1";
  rc = sqlite3_prepare_v2(db, query, -1, &stmt, 0);

  if (rc != SQLITE_OK) {
    log_error("%s -- Failed to create prepared statement: %s", __func__, sqlite3_errmsg(db));
    return NULL;
  }

  sqlite3_bind_text(stmt, 1, address, -1, NULL);
  rc = sqlite3_step(stmt);

  cJSON *json = NULL;
  if (rc == SQLITE_ROW) {
    json =  cJSON_CreateObject();

    cJSON_AddNumberToObject(json, "serial", sqlite3_column_int(stmt, 0 ));
    cJSON_AddStringToObject(json, "address", (char*)sqlite3_column_text(stmt, 1 ));
    cJSON_AddNumberToObject(json, "offset", sqlite3_column_int(stmt, 2 ));
    cJSON_AddStringToObject(json, "account", (char*)sqlite3_column_text(stmt, 3 ));
    cJSON_AddNumberToObject(json, "is_change", sqlite3_column_int(stmt, 4 ));
    cJSON_AddStringToObject(json, "created_at", (char*)sqlite3_column_text(stmt, 5));
  }

  sqlite3_finalize(stmt);
  return json;
}

cJSON* get_next_fresh_address(sqlite3* db, const char* username) {
  enforce_max_length_null(strlen(username))
  sqlite3_stmt* stmt;
  int rc;

  char* query = "SELECT * FROM address WHERE is_change=0 AND account=? ORDER BY offset ASC LIMIT 1";
  rc = sqlite3_prepare_v2(db, query, -1, &stmt, 0);

  if (rc != SQLITE_OK) {
    log_error("%s -- Failed to create prepared statement: %s", __func__, sqlite3_errmsg(db));
    return NULL;
  }

  sqlite3_bind_text(stmt, 1, username, -1, NULL);
  rc = sqlite3_step(stmt);

  cJSON *json = NULL;
  if (rc == SQLITE_ROW) {
    json =  cJSON_CreateObject();

    cJSON_AddNumberToObject(json, "serial", sqlite3_column_int(stmt, 0 ));
    cJSON_AddStringToObject(json, "address", (char*)sqlite3_column_text(stmt, 1 ));
    cJSON_AddNumberToObject(json, "offset", sqlite3_column_int(stmt, 2 ));
    cJSON_AddStringToObject(json, "account", (char*)sqlite3_column_text(stmt, 3 ));
    cJSON_AddNumberToObject(json, "is_change", sqlite3_column_int(stmt, 4 ));
    cJSON_AddStringToObject(json, "created_at", (char*)sqlite3_column_text(stmt, 5));
  }

  sqlite3_finalize(stmt);
  return json;
}

int mark_address_is_change_address(sqlite3* db, const char* address) {
  enforce_max_length(strlen(address))
  sqlite3_stmt* stmt;
  int rc;

  char* query = "UPDATE address SET is_change=1 WHERE address=?";
  rc = sqlite3_prepare_v2(db, query, -1, &stmt, 0);

  if (rc != SQLITE_OK) {
    log_error("%s -- Failed to create prepared statement: %s", __func__, sqlite3_errmsg(db));
    return -1;
  }

  sqlite3_bind_text(stmt, 1, address, -1, NULL);
  rc = sqlite3_step(stmt);

  if (rc != SQLITE_DONE) {
    log_error("%s execution failed: %s", __func__, sqlite3_errmsg(db));
    sqlite3_reset(stmt);
    sqlite3_finalize(stmt);
    return -1;
  }
  sqlite3_finalize(stmt);
  return 0;
}