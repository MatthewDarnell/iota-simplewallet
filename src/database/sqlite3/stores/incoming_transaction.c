//
// Created by matth on 2/18/2020.
//

#include <stdio.h>
#include <string.h>
#include "../../../iota-simplewallet.h"
#include "incoming_transaction.h"

#define enforce_max_length(len) if(len > 1024) return -1;
#define enforce_max_length_null(len) if(len > 1024) return NULL;
int create_incoming_transaction(sqlite3* db, const char* address, uint64_t amount, const char* bundle, const char* hash, const char* time, int confirmed) {
  enforce_max_length(strlen(address) + 30 + strlen(bundle) + strlen(hash) + strlen(time) + 10)
  sqlite3_stmt* stmt;
  int rc;

  char* query = "INSERT INTO incoming_transaction(address, amount, bundle, hash, time, confirmed) VALUES(?, ?, ?, ?, ?, ?)";
  rc = sqlite3_prepare_v2(db, query, -1, &stmt, 0);

  if (rc != SQLITE_OK) {
    log_wallet_error("%s -- Failed to create prepared statement: %s", __func__, sqlite3_errmsg(db));
    return -1;
  }

  sqlite3_bind_text(stmt, 1, address, -1, NULL);
  sqlite3_bind_int64(stmt, 2, (int64_t)amount);
  sqlite3_bind_text(stmt, 3, bundle, -1, NULL);
  sqlite3_bind_text(stmt, 4, hash, -1, NULL);
  sqlite3_bind_text(stmt, 5, time, -1, NULL);
  sqlite3_bind_int(stmt, 6, confirmed);

  int count = 0;
  while((rc = sqlite3_step(stmt)) == SQLITE_BUSY) {
    if(count > 5) {
      break;
    }
    count++;
    sqlite3_sleep(100);
  }
  if(sqlite3_extended_errcode(db) == SQLITE_CONSTRAINT_UNIQUE) {
    sqlite3_reset(stmt);
    sqlite3_finalize(stmt);
    return -1;
  } else if(rc != SQLITE_DONE) {
    log_wallet_error("%s execution failed: %s", __func__, sqlite3_errmsg(db));
    sqlite3_reset(stmt);
    sqlite3_finalize(stmt);
    return -2;
  }
  sqlite3_finalize(stmt);
  log_wallet_info( "Created new incoming transaction <%s> for address %s", hash, address);
  return 0;
}



cJSON* get_incoming_transaction_by_address(sqlite3* db, const char* address) {
  enforce_max_length_null(strlen(address))
  sqlite3_stmt* stmt;
  int rc;

  char* query = "SELECT * FROM incoming_transaction WHERE address=? LIMIT 1";
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
    cJSON_AddStringToObject(json, "time", (char*)sqlite3_column_text(stmt, 1 ));

    uint64_t amount = sqlite3_column_int64(stmt, 2);
    char str_amount[64] = { 0 };

    #ifdef WIN32
    snprintf(str_amount, 64, "%I64d", amount);
    #else
    snprintf(str_amount, 64, "%lld", amount);
    #endif

    cJSON_AddStringToObject(json, "amount", str_amount);

    cJSON_AddStringToObject(json, "address", (char*)sqlite3_column_text(stmt, 3 ));
    cJSON_AddNumberToObject(json, "confirmed", sqlite3_column_int(stmt, 4 ));
    cJSON_AddStringToObject(json, "bundle", (char*)sqlite3_column_text(stmt, 5 ));
    cJSON_AddStringToObject(json, "hash", (char*)sqlite3_column_text(stmt, 6 ));
    cJSON_AddStringToObject(json, "created_at", (char*)sqlite3_column_text(stmt, 7));
  }

  sqlite3_finalize(stmt);
  return json;
}

cJSON* get_incoming_transaction_hash(sqlite3* db, const char* hash) {
  enforce_max_length_null(strlen(hash))
  sqlite3_stmt* stmt;
  int rc;

  char* query = "SELECT * FROM incoming_transaction WHERE hash=? LIMIT 1";
  rc = sqlite3_prepare_v2(db, query, -1, &stmt, 0);

  if (rc != SQLITE_OK) {
    log_wallet_error("%s -- Failed to create prepared statement: %s", __func__, sqlite3_errmsg(db));
    return NULL;
  }

  sqlite3_bind_text(stmt, 1, hash, -1, NULL);
  rc = sqlite3_step(stmt);

  cJSON *json = NULL;
  if (rc == SQLITE_ROW) {
    json =  cJSON_CreateObject();

    cJSON_AddNumberToObject(json, "serial", sqlite3_column_int(stmt, 0 ));
    cJSON_AddStringToObject(json, "time", (char*)sqlite3_column_text(stmt, 1 ));

    uint64_t amount = sqlite3_column_int64(stmt, 2);
    char str_amount[64] = { 0 };

#ifdef WIN32
    snprintf(str_amount, 64, "%I64d", amount);
#else
    snprintf(str_amount, 64, "%lld", amount);
#endif

    cJSON_AddStringToObject(json, "amount", str_amount);

    cJSON_AddStringToObject(json, "address", (char*)sqlite3_column_text(stmt, 3 ));
    cJSON_AddNumberToObject(json, "confirmed", sqlite3_column_int(stmt, 4 ));
    cJSON_AddStringToObject(json, "bundle", (char*)sqlite3_column_text(stmt, 5 ));
    cJSON_AddStringToObject(json, "hash", (char*)sqlite3_column_text(stmt, 6 ));
    cJSON_AddStringToObject(json, "created_at", (char*)sqlite3_column_text(stmt, 7));
  }

  sqlite3_finalize(stmt);
  return json;
}

cJSON* get_all_incoming_transactions(sqlite3* db, const char* username, uint32_t offset, uint32_t limit) {
  sqlite3_stmt* stmt;
  int rc;

  char* query = "SELECT it.amount, it.address, it.hash, it.bundle, it.confirmed, it.time"
                " FROM incoming_transaction it"
                " INNER JOIN address a on it.address = a.address"
                " INNER JOIN  account a2 on a.account = a2.username"
                " WHERE a2.username=?"
                " ORDER BY it.time ASC"
                " LIMIT ? OFFSET ?";

  rc = sqlite3_prepare_v2(db, query, -1, &stmt, 0);

  if (rc != SQLITE_OK) {
    log_wallet_error("%s -- Failed to create prepared statement: %s", __func__, sqlite3_errmsg(db));
    return NULL;
  }

  sqlite3_bind_text(stmt, 1, username, -1, NULL);
  sqlite3_bind_int(stmt, 2, limit);
  sqlite3_bind_int(stmt, 3, offset);
  rc = sqlite3_step(stmt);

  cJSON *json = cJSON_CreateArray();

  while(rc == SQLITE_ROW) {
    cJSON *row =  cJSON_CreateObject();
    cJSON_AddStringToObject(row, "amount", (char*)sqlite3_column_text(stmt, 0));
    cJSON_AddStringToObject(row, "address", (char*)sqlite3_column_text(stmt, 1));
    cJSON_AddStringToObject(row, "hash", (char*)sqlite3_column_text(stmt, 2 ));
    cJSON_AddStringToObject(row, "bundle", (char*)sqlite3_column_text(stmt, 3 ));
    cJSON_AddNumberToObject(row, "confirmed", sqlite3_column_int(stmt, 4 ));
    cJSON_AddStringToObject(row, "time", (char*)sqlite3_column_text(stmt, 5 ));
    cJSON_AddItemToArray(json, row);
    rc = sqlite3_step(stmt);
  }

  sqlite3_finalize(stmt);
  return json;
}

cJSON* get_unspents_by_username(sqlite3* db, const char* username) {
  enforce_max_length_null(strlen(username))
  sqlite3_stmt* stmt;
  int rc;

  char* query = "SELECT it.amount, it.address, it.hash"
                " FROM incoming_transaction it"
                " INNER JOIN address a on it.address = a.address"
                " INNER JOIN  account a2 on a.account = a2.username"
                " WHERE it.confirmed = 1"
                " AND a2.username=?";
  rc = sqlite3_prepare_v2(db, query, -1, &stmt, 0);

  if (rc != SQLITE_OK) {
    log_wallet_error("%s -- Failed to create prepared statement: %s", __func__, sqlite3_errmsg(db));
    return NULL;
  }
  sqlite3_bind_text(stmt, 1, username, -1, NULL);

  rc = sqlite3_step(stmt);

  cJSON *json = cJSON_CreateArray();

  while(rc == SQLITE_ROW) {
    cJSON *row =  cJSON_CreateObject();
    cJSON_AddStringToObject(row, "amount", (char*)sqlite3_column_text(stmt, 0));
    cJSON_AddStringToObject(row, "address", (char*)sqlite3_column_text(stmt, 1));
    cJSON_AddStringToObject(row, "hash", (char*)sqlite3_column_text(stmt, 2 ));
    cJSON_AddItemToArray(json, row);
    rc = sqlite3_step(stmt);
  }

  sqlite3_finalize(stmt);
  return json;
}

int mark_incoming_transaction_confirmed(sqlite3* db, const char* hash) {
  enforce_max_length( + strlen(hash))
  sqlite3_stmt* stmt;
  int rc;

  char* query = "UPDATE incoming_transaction SET confirmed=1 WHERE hash=? AND confirmed IS NOT 1";
  rc = sqlite3_prepare_v2(db, query, -1, &stmt, 0);

  if (rc != SQLITE_OK) {
    log_wallet_error( "%s -- Failed to create prepared statement: %s", __func__, sqlite3_errmsg(db));
    return -1;
  }

  sqlite3_bind_text(stmt, 1, hash, -1, NULL);

  int count = 0;
  while((rc = sqlite3_step(stmt)) == SQLITE_BUSY) {
    if(count > 5) {
      break;
    }
    count++;
    sqlite3_sleep(100);
  }

  int ret_val = 0;

  if (rc != SQLITE_DONE) {
    log_wallet_error("%s execution failed: %s", __func__, sqlite3_errmsg(db));
    sqlite3_reset(stmt);
    ret_val = -1;
  } else if(sqlite3_changes(db) < 1) {
    log_wallet_debug("hash %s not marked confirmed", hash);
    ret_val = -1;
  } else {
      log_wallet_info("Confirming Incoming Transaction. (hash=<%s>)", hash);
  }
  sqlite3_finalize(stmt);
  return ret_val;
}