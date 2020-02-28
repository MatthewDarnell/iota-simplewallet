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
    log_wallet_error("%s -- Failed to create prepared statement: %s", __func__, sqlite3_errmsg(db));
    return -1;
  }

  sqlite3_bind_text(stmt, 1, address, -1, NULL);
  sqlite3_bind_int(stmt, 2, offset);
  sqlite3_bind_text(stmt, 3, username, -1, NULL);

  rc = sqlite3_step(stmt);

  if (rc != SQLITE_DONE) {
    log_wallet_error("%s execution failed: %s", __func__, sqlite3_errmsg(db));
    sqlite3_reset(stmt);
    sqlite3_finalize(stmt);
    return -1;
  }
  sqlite3_finalize(stmt);
  log_wallet_debug("Created new address %s for user %s", address, username);
  return 0;
}

cJSON* get_address_by_address(sqlite3* db, const char* address) {
  enforce_max_length_null(strlen(address))
  sqlite3_stmt* stmt;
  int rc;

  char* query = "SELECT * FROM address WHERE address=? LIMIT 1";
  rc = sqlite3_prepare_v2(db, query, -1, &stmt, 0);

  if (rc != SQLITE_OK) {
    log_wallet_error("%s -- Failed to create prepared statement: %s", __func__, sqlite3_errmsg(db));
    return NULL;
  }

  sqlite3_bind_text(stmt, 1, address, -1, NULL);
  rc = sqlite3_step(stmt);

  cJSON *json = NULL;
  if (rc == SQLITE_ROW) {
    json =  cJSON_CreateObject();

    cJSON_AddNumberToObject(json, "serial", sqlite3_column_int(stmt, 0 ));
    cJSON_AddStringToObject(json, "address", (char*)sqlite3_column_text(stmt, 1 ));
    cJSON_AddStringToObject(json, "balance", (char*)sqlite3_column_text(stmt, 2 ));
    cJSON_AddNumberToObject(json, "offset", sqlite3_column_int(stmt, 3 ));
    cJSON_AddStringToObject(json, "account", (char*)sqlite3_column_text(stmt, 4 ));
    cJSON_AddNumberToObject(json, "is_change", sqlite3_column_int(stmt, 5 ));
    cJSON_AddStringToObject(json, "created_at", (char*)sqlite3_column_text(stmt, 6));
  }

  sqlite3_finalize(stmt);
  return json;
}

cJSON* get_next_fresh_address(sqlite3* db, const char* username) {
  enforce_max_length_null(strlen(username))
  sqlite3_stmt* stmt;
  int rc;

  char* query = "SELECT address FROM address WHERE is_change=0 AND account=? ORDER BY offset ASC LIMIT 1";
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

    cJSON_AddStringToObject(json, "address", (char*)sqlite3_column_text(stmt, 0 ));
  }

  sqlite3_finalize(stmt);
  return json;
}

cJSON* get_deposit_addresses(sqlite3* db) {
  sqlite3_stmt* stmt;
  int rc;

  char* query = "SELECT *"
                " FROM address a"
                " WHERE a.address NOT IN"
                " (SELECT input AS address FROM input_to_output)"
  ;
  rc = sqlite3_prepare_v2(db, query, -1, &stmt, 0);

  if (rc != SQLITE_OK) {
    log_wallet_error("%s -- Failed to create prepared statement: %s", __func__, sqlite3_errmsg(db));
    return NULL;
  }

  rc = sqlite3_step(stmt);

  cJSON *json = cJSON_CreateArray();

  while(rc == SQLITE_ROW) {
    cJSON *row =  cJSON_CreateObject();
    cJSON_AddNumberToObject(row, "serial", sqlite3_column_int(stmt, 0 ));
    cJSON_AddStringToObject(row, "address", (char*)sqlite3_column_text(stmt, 1 ));
    cJSON_AddStringToObject(row, "balance", (char*)sqlite3_column_text(stmt, 2 ));
    cJSON_AddNumberToObject(row, "offset", sqlite3_column_int(stmt, 3 ));
    cJSON_AddStringToObject(row, "account", (char*)sqlite3_column_text(stmt, 4 ));
    cJSON_AddNumberToObject(row, "is_change", sqlite3_column_int(stmt, 5 ));
    cJSON_AddStringToObject(row, "created_at", (char*)sqlite3_column_text(stmt, 6));
    cJSON_AddItemToArray(json, row);
    rc = sqlite3_step(stmt);
  }

  sqlite3_finalize(stmt);
  return json;
}

int32_t set_address_balance(sqlite3* db, const char* address, const char* balance) {
  enforce_max_length(strlen(address))
  enforce_max_length(strlen(balance))
  sqlite3_stmt* stmt;
  int rc;

  char* query = "UPDATE address SET balance=? WHERE address=? AND balance!=?";
  rc = sqlite3_prepare_v2(db, query, -1, &stmt, 0);

  if (rc != SQLITE_OK) {
    log_wallet_error("%s -- Failed to create prepared statement: %s", __func__, sqlite3_errmsg(db));
    return -1;
  }

  sqlite3_bind_text(stmt, 1, balance, -1, NULL);
  sqlite3_bind_text(stmt, 2, address, -1, NULL);
  sqlite3_bind_text(stmt, 3, balance, -1, NULL);
  rc = sqlite3_step(stmt);

  if (rc != SQLITE_DONE) {
    log_wallet_error("%s execution failed: %s", __func__, sqlite3_errmsg(db));
    sqlite3_reset(stmt);
    sqlite3_finalize(stmt);
    return -1;
  } else if(sqlite3_changes(db) > 0) {
    log_wallet_info("Updating Address balance: <%s> --> (%s i)", address, balance);
  }
  sqlite3_finalize(stmt);
  return 0;
}

int32_t get_num_fresh_addresses(sqlite3* db, const char* username) {
  enforce_max_length(strlen(username))
  sqlite3_stmt* stmt;
  int rc;

  char* query = "SELECT COUNT(*) as num "
                " FROM address a"
                " WHERE account=? AND"
                " is_change=0 AND"
                " a.address NOT IN "
                " (SELECT input AS address FROM input_to_output)"
                ;
  rc = sqlite3_prepare_v2(db, query, -1, &stmt, 0);

  if (rc != SQLITE_OK) {
    log_wallet_error("%s -- Failed to create prepared statement: %s", __func__, sqlite3_errmsg(db));
    return -1;
  }

  sqlite3_bind_text(stmt, 1, username, -1, NULL);
  rc = sqlite3_step(stmt);

  int num = -1;
  if (rc == SQLITE_ROW) {
    num = sqlite3_column_int(stmt, 0);
  }

  sqlite3_finalize(stmt);
  return num;
}


int32_t get_latest_offset(sqlite3* db, const char* username) {
  enforce_max_length(strlen(username))
  sqlite3_stmt* stmt;
  int rc;

  char* query = "SELECT offset FROM address WHERE account=? ORDER BY offset DESC LIMIT 1";
  rc = sqlite3_prepare_v2(db, query, -1, &stmt, 0);

  if (rc != SQLITE_OK) {
    log_wallet_error("%s -- Failed to create prepared statement: %s", __func__, sqlite3_errmsg(db));
    return -1;
  }

  sqlite3_bind_text(stmt, 1, username, -1, NULL);
  rc = sqlite3_step(stmt);

  int offset = -1;
  if (rc == SQLITE_ROW) {
    offset = sqlite3_column_int(stmt, 0 );
  } else {
    offset = 0;
  }

  sqlite3_finalize(stmt);
  return offset;
}


int mark_address_is_change_address(sqlite3* db, const char* address) {
  enforce_max_length(strlen(address))
  sqlite3_stmt* stmt;
  int rc;

  char* query = "UPDATE address SET is_change=1 WHERE address=?";
  rc = sqlite3_prepare_v2(db, query, -1, &stmt, 0);

  if (rc != SQLITE_OK) {
    log_wallet_error("%s -- Failed to create prepared statement: %s", __func__, sqlite3_errmsg(db));
    return -1;
  }

  sqlite3_bind_text(stmt, 1, address, -1, NULL);
  rc = sqlite3_step(stmt);

  if (rc != SQLITE_DONE) {
    log_wallet_error("%s execution failed: %s", __func__, sqlite3_errmsg(db));
    sqlite3_reset(stmt);
    sqlite3_finalize(stmt);
    return -1;
  }
  sqlite3_finalize(stmt);
  return 0;
}