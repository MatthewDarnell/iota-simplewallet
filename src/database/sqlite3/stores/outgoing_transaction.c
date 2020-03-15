//
// Created by matth on 2/18/2020.
//

#include <stdio.h>
#include <string.h>
#include "../../../iota-simplewallet.h"
#include "outgoing_transaction.h"

#define enforce_max_length(len) if(len > 1024) return -1;
#define enforce_max_length_null(len) if(len > 1024) return NULL;
int create_outgoing_transaction(sqlite3* db, const char* dest_address, const char* change_address, uint64_t amount, const char* trytes) {
  sqlite3_stmt* stmt;
  int rc;

  char* query = "INSERT INTO outgoing_transaction(dest_address, change_address, amount, trytes) VALUES(?, ?, ?, ?)";
  rc = sqlite3_prepare_v2(db, query, -1, &stmt, 0);

  if (rc != SQLITE_OK) {
    log_wallet_error( "%s -- Failed to create prepared statement: %s", __func__, sqlite3_errmsg(db));
    return -1;
  }

  sqlite3_bind_text(stmt, 1, dest_address, -1, NULL);
  sqlite3_bind_text(stmt, 2, change_address, -1, NULL);
  sqlite3_bind_int64(stmt, 3, (int64_t)amount);
  sqlite3_bind_text(stmt, 4, trytes, -1, NULL);

  int count = 0;
  while((rc = sqlite3_step(stmt)) == SQLITE_BUSY) {
    if(count > 5) {
      break;
    }
    count++;
    sqlite3_sleep(100);
  }

  if (rc != SQLITE_DONE) {
    log_wallet_error("%s execution failed: %s", __func__, sqlite3_errmsg(db));
    sqlite3_reset(stmt);
    sqlite3_finalize(stmt);
    return -1;
  }
  sqlite3_finalize(stmt);
  #ifdef WIN32
  log_wallet_info("Created new outgoing transaction to address <%s> (%I64d i)", dest_address, amount);
  #else
  log_wallet_info("Created new outgoing transaction to address <%s> (%lld i)", dest_address, amount);
  #endif
  return 0;
}


cJSON* get_outgoing_transaction_by_serial(sqlite3* db, int serial) {
  enforce_max_length_null(10)
  sqlite3_stmt* stmt;
  int rc;

  char* query = "SELECT * FROM outgoing_transaction WHERE serial=?";
  rc = sqlite3_prepare_v2(db, query, -1, &stmt, 0);

  if (rc != SQLITE_OK) {
    log_wallet_error( "%s -- Failed to create prepared statement: %s", __func__, sqlite3_errmsg(db));
    return NULL;
  }

  sqlite3_bind_int(stmt, 1, serial);

  int count = 0;
  while((rc = sqlite3_step(stmt)) == SQLITE_BUSY) {
    if(count > 5) {
      break;
    }
    count++;
    sqlite3_sleep(100);
  }

  cJSON *json = NULL;
  if (rc == SQLITE_ROW) {
    json =  cJSON_CreateObject();

    cJSON_AddNumberToObject(json, "serial", sqlite3_column_int(stmt, 0 ));

    uint64_t amount = sqlite3_column_int64(stmt, 1);
    char str_amount[64] = { 0 };

    #ifdef WIN32
        snprintf(str_amount, 64, "%I64d", amount);
    #else
        snprintf(str_amount, 64, "%lld", amount);
    #endif
    cJSON_AddStringToObject(json, "amount", str_amount);


    cJSON_AddStringToObject(json, "dest_address", (char*)sqlite3_column_text(stmt, 2 ));
    cJSON_AddStringToObject(json, "change_address", (char*)sqlite3_column_text(stmt, 3 ));
    cJSON_AddNumberToObject(json, "confirmed", sqlite3_column_int(stmt, 4 ));
    cJSON_AddStringToObject(json, "trytes", (char*)sqlite3_column_text(stmt, 5 ));
    cJSON_AddNumberToObject(json, "sent", sqlite3_column_int(stmt, 6 ));
    cJSON_AddStringToObject(json, "bundle", (char*)sqlite3_column_text(stmt, 7 ));
    cJSON_AddStringToObject(json, "hash", (char*)sqlite3_column_text(stmt, 8 ));
    cJSON_AddStringToObject(json, "created_at", (char*)sqlite3_column_text(stmt, 9));
  }

  sqlite3_finalize(stmt);
  return json;
}

cJSON* get_outgoing_transaction_hash(sqlite3* db, char* hash) {
  enforce_max_length_null(strlen(hash))
  sqlite3_stmt* stmt;
  int rc;

  char* query = "SELECT * FROM outgoing_transaction WHERE hash=?";
  rc = sqlite3_prepare_v2(db, query, -1, &stmt, 0);

  if (rc != SQLITE_OK) {
    log_wallet_error( "%s -- Failed to create prepared statement: %s", __func__, sqlite3_errmsg(db));
    return NULL;
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

  cJSON *json = NULL;
  if (rc == SQLITE_ROW) {
    json =  cJSON_CreateObject();

    cJSON_AddNumberToObject(json, "serial", sqlite3_column_int(stmt, 0 ));

    uint64_t amount = sqlite3_column_int64(stmt, 1);
    char str_amount[64] = { 0 };

#ifdef WIN32
    snprintf(str_amount, 64, "%I64d", amount);
#else
    snprintf(str_amount, 64, "%lld", amount);
#endif
    cJSON_AddStringToObject(json, "amount", str_amount);


    cJSON_AddStringToObject(json, "dest_address", (char*)sqlite3_column_text(stmt, 2 ));
    cJSON_AddStringToObject(json, "change_address", (char*)sqlite3_column_text(stmt, 3 ));
    cJSON_AddNumberToObject(json, "confirmed", sqlite3_column_int(stmt, 4 ));
    cJSON_AddStringToObject(json, "trytes", (char*)sqlite3_column_text(stmt, 5 ));
    cJSON_AddNumberToObject(json, "sent", sqlite3_column_int(stmt, 6 ));
    cJSON_AddStringToObject(json, "bundle", (char*)sqlite3_column_text(stmt, 7 ));
    cJSON_AddStringToObject(json, "hash", (char*)sqlite3_column_text(stmt, 8 ));
    cJSON_AddStringToObject(json, "created_at", (char*)sqlite3_column_text(stmt, 9));
  }

  sqlite3_finalize(stmt);
  return json;
}

cJSON* get_all_outgoing_transactions(sqlite3* db, const char* username, uint32_t offset, uint32_t limit) {
  sqlite3_stmt* stmt;
  int rc;

  char* query = "SELECT ot.amount, ot.dest_address as address, ot.change_address, ot.confirmed, ot.sent, ot.bundle, ot.hash, ot.created_at as time"
                " FROM outgoing_transaction ot"
                " INNER JOIN address a on ot.change_address = a.address"
                " INNER JOIN  account a2 on a.account = a2.username"
                " WHERE a2.username=?"
                " ORDER BY ot.created_at ASC"
                " LIMIT ? OFFSET ?";

  rc = sqlite3_prepare_v2(db, query, -1, &stmt, 0);

  if (rc != SQLITE_OK) {
    log_wallet_error( "%s -- Failed to create prepared statement: %s", __func__, sqlite3_errmsg(db));
    return NULL;
  }

  sqlite3_bind_text(stmt, 1, username, -1, NULL);
  sqlite3_bind_int(stmt, 2, limit);
  sqlite3_bind_int(stmt, 3, offset);

  int count = 0;
  while((rc = sqlite3_step(stmt)) == SQLITE_BUSY) {
    if(count > 5) {
      break;
    }
    count++;
    sqlite3_sleep(100);
  }

  cJSON *json = cJSON_CreateArray();

  while(rc == SQLITE_ROW) {
    cJSON *row =  cJSON_CreateObject();


    uint64_t amount = sqlite3_column_int64(stmt, 0);
    char str_amount[64] = { 0 };

#ifdef WIN32
    snprintf(str_amount, 64, "%I64d", amount);
#else
    snprintf(str_amount, 64, "%lld", amount);
#endif
    cJSON_AddStringToObject(row, "amount", str_amount);
    cJSON_AddStringToObject(row, "address", (char*)sqlite3_column_text(stmt, 1));
    cJSON_AddStringToObject(row, "change_address", (char*)sqlite3_column_text(stmt, 2 ));
    cJSON_AddNumberToObject(row, "confirmed", sqlite3_column_int(stmt, 3 ));
    cJSON_AddNumberToObject(row, "sent", sqlite3_column_int(stmt, 4 ));
    cJSON_AddStringToObject(row, "bundle", (char*)sqlite3_column_text(stmt, 5 ));
    cJSON_AddStringToObject(row, "hash", (char*)sqlite3_column_text(stmt, 6 ));
    cJSON_AddStringToObject(row, "time", (char*)sqlite3_column_text(stmt, 7));

    cJSON_AddItemToArray(json, row);
    rc = sqlite3_step(stmt);
  }

  sqlite3_finalize(stmt);
  return json;
}

cJSON* get_all_unsent_outgoing_transactions(sqlite3* db) {
  sqlite3_stmt* stmt;
  int rc;

  char* query = "SELECT * FROM outgoing_transaction WHERE sent=0";
  rc = sqlite3_prepare_v2(db, query, -1, &stmt, 0);

  if (rc != SQLITE_OK) {
    log_wallet_error( "%s -- Failed to create prepared statement: %s", __func__, sqlite3_errmsg(db));
    return NULL;
  }

  int count = 0;
  while((rc = sqlite3_step(stmt)) == SQLITE_BUSY) {
    if(count > 5) {
      break;
    }
    count++;
    sqlite3_sleep(100);
  }

  cJSON *json = cJSON_CreateArray();

  while(rc == SQLITE_ROW) {
    cJSON *row =  cJSON_CreateObject();

    cJSON_AddNumberToObject(row, "serial", sqlite3_column_int(stmt, 0 ));

    uint64_t amount = sqlite3_column_int64(stmt, 1);
    char str_amount[64] = { 0 };

#ifdef WIN32
    snprintf(str_amount, 64, "%I64d", amount);
#else
    snprintf(str_amount, 64, "%lld", amount);
#endif
    cJSON_AddStringToObject(row, "amount", str_amount);
    cJSON_AddStringToObject(row, "dest_address", (char*)sqlite3_column_text(stmt, 2));
    cJSON_AddStringToObject(row, "change_address", (char*)sqlite3_column_text(stmt, 3 ));
    cJSON_AddNumberToObject(row, "confirmed", sqlite3_column_int(stmt, 4 ));
    cJSON_AddStringToObject(row, "trytes", (char*)sqlite3_column_text(stmt, 5 ));
    cJSON_AddNumberToObject(row, "sent", sqlite3_column_int(stmt, 6 ));
    cJSON_AddStringToObject(row, "bundle", (char*)sqlite3_column_text(stmt, 7 ));
    cJSON_AddStringToObject(row, "hash", (char*)sqlite3_column_text(stmt, 8 ));
    cJSON_AddStringToObject(row, "created_at", (char*)sqlite3_column_text(stmt, 9));

    cJSON_AddItemToArray(json, row);
    rc = sqlite3_step(stmt);
  }

  sqlite3_finalize(stmt);
  return json;
}

cJSON* get_all_unconfirmed_outgoing_transactions(sqlite3* db) {
  sqlite3_stmt* stmt;
  int rc;

  char* query = "SELECT * FROM outgoing_transaction WHERE sent=1 AND confirmed=0";
  rc = sqlite3_prepare_v2(db, query, -1, &stmt, 0);

  if (rc != SQLITE_OK) {
    log_wallet_error( "%s -- Failed to create prepared statement: %s", __func__, sqlite3_errmsg(db));
    return NULL;
  }

  int count = 0;
  while((rc = sqlite3_step(stmt)) == SQLITE_BUSY) {
    if(count > 5) {
      break;
    }
    count++;
    sqlite3_sleep(100);
  }

  cJSON *json = cJSON_CreateArray();

  while(rc == SQLITE_ROW) {
    cJSON *row =  cJSON_CreateObject();

    cJSON_AddNumberToObject(row, "serial", sqlite3_column_int(stmt, 0 ));

    uint64_t amount = sqlite3_column_int64(stmt, 1);
    char str_amount[64] = { 0 };

#ifdef WIN32
    snprintf(str_amount, 64, "%I64d", amount);
#else
    snprintf(str_amount, 64, "%lld", amount);
#endif
    cJSON_AddStringToObject(row, "amount", str_amount);

    cJSON_AddStringToObject(row, "dest_address", (char*)sqlite3_column_text(stmt, 2 ));
    cJSON_AddStringToObject(row, "change_address", (char*)sqlite3_column_text(stmt, 3 ));
    cJSON_AddNumberToObject(row, "confirmed", sqlite3_column_int(stmt, 4 ));
    cJSON_AddStringToObject(row, "trytes", (char*)sqlite3_column_text(stmt, 5 ));
    cJSON_AddNumberToObject(row, "sent", sqlite3_column_int(stmt, 6 ));
    cJSON_AddStringToObject(row, "bundle", (char*)sqlite3_column_text(stmt, 7 ));
    cJSON_AddStringToObject(row, "hash", (char*)sqlite3_column_text(stmt, 8 ));
    cJSON_AddStringToObject(row, "created_at", (char*)sqlite3_column_text(stmt, 9));

    cJSON_AddItemToArray(json, row);
    rc = sqlite3_step(stmt);
  }

  sqlite3_finalize(stmt);
  return json;
}

int mark_outgoing_transaction_confirmed(sqlite3* db, int serial, const char* bundle, const char* hash) {
  enforce_max_length(strlen(bundle) + strlen(hash))
  sqlite3_stmt* stmt;
  int rc;

  char* query = "UPDATE outgoing_transaction SET confirmed=1, trytes=\"\" WHERE serial=? AND bundle=? AND hash=?";
  rc = sqlite3_prepare_v2(db, query, -1, &stmt, 0);

  if (rc != SQLITE_OK) {
    log_wallet_error( "%s -- Failed to create prepared statement: %s", __func__, sqlite3_errmsg(db));
    return -1;
  }

  sqlite3_bind_int(stmt, 1, serial);
  sqlite3_bind_text(stmt, 2, bundle, -1, NULL);
  sqlite3_bind_text(stmt, 3, hash, -1, NULL);

  int count = 0;
  while((rc = sqlite3_step(stmt)) == SQLITE_BUSY) {
    if(count > 5) {
      break;
    }
    count++;
    sqlite3_sleep(100);
  }

  if (rc != SQLITE_DONE) {
    log_wallet_error("%s execution failed: %s", __func__, sqlite3_errmsg(db));
    sqlite3_reset(stmt);
    sqlite3_finalize(stmt);
    return -1;
  } else {
    log_wallet_debug("Confirming Outgoing Transaction (%d). (hash=<%s>) (bundle=<%s>)\n", serial, hash, bundle);
  }
  sqlite3_finalize(stmt);
  return 0;
}

int mark_outgoing_transaction_sent(sqlite3* db, int serial, const char* bundle, const char* hash) {
  enforce_max_length(strlen(bundle) + strlen(hash))
  sqlite3_stmt* stmt;
  int rc;

  char* query = "UPDATE outgoing_transaction SET sent=1, bundle=?, hash=?  WHERE serial=?";
  rc = sqlite3_prepare_v2(db, query, -1, &stmt, 0);

  if (rc != SQLITE_OK) {
    log_wallet_error( "%s -- Failed to create prepared statement: %s", __func__, sqlite3_errmsg(db));
    return -1;
  }

  sqlite3_bind_text(stmt, 1, bundle, -1, NULL);
  sqlite3_bind_text(stmt, 2, hash, -1, NULL);
  sqlite3_bind_int(stmt, 3, serial);


  int count = 0;
  while((rc = sqlite3_step(stmt)) == SQLITE_BUSY) {
    if(count > 5) {
      break;
    }
    count++;
    sqlite3_sleep(100);
  }

  if (rc != SQLITE_DONE) {
    log_wallet_error("%s execution failed: %s", __func__, sqlite3_errmsg(db));
    sqlite3_reset(stmt);
    sqlite3_finalize(stmt);
    return -1;
  } else {
    log_wallet_debug("Marked Outgoing Transaction (%d) as sent. (hash=<%s>) (bundle=<%s>)\n", serial, hash, bundle);
  }
  sqlite3_finalize(stmt);
  return 0;
}