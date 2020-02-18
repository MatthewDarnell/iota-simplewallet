//
// Created by matth on 2/18/2020.
//

#include <stdio.h>
#include "incoming_transaction.h"

int create_incoming_transaction(sqlite3* db, const char* address, uint64_t amount, const char* bundle, const char* hash, const char* time, int confirmed) {
  sqlite3_stmt* stmt;
  int rc;

  char* query = "INSERT INTO incoming_transaction(address, amount, bundle, hash, time, confirmed) VALUES(?, ?, ?, ?, ?, ?)";
  rc = sqlite3_prepare_v2(db, query, -1, &stmt, 0);

  if (rc != SQLITE_OK) {
    fprintf(stderr, "%s -- Failed to create prepared statement: %s\n", __func__, sqlite3_errmsg(db));
    return -1;
  }

  sqlite3_bind_text(stmt, 1, address, -1, NULL);
  sqlite3_bind_int64(stmt, 2, (int64_t)amount);
  sqlite3_bind_text(stmt, 3, bundle, -1, NULL);
  sqlite3_bind_text(stmt, 4, hash, -1, NULL);
  sqlite3_bind_text(stmt, 5, time, -1, NULL);
  sqlite3_bind_int(stmt, 6, confirmed);

  rc = sqlite3_step(stmt);

  if (rc != SQLITE_DONE) {
    fprintf(stderr,"%s execution failed: %s\n", __func__, sqlite3_errmsg(db));
    sqlite3_reset(stmt);
    sqlite3_finalize(stmt);
    return -1;
  }
  sqlite3_finalize(stmt);
  fprintf(stdout, "Created new incoming transaction <%s> for address %s\n", hash, address);
  return 0;
}



cJSON* get_incoming_transaction_by_address(sqlite3* db, const char* address) {
  sqlite3_stmt* stmt;
  int rc;

  char* query = "SELECT * FROM incoming_transaction WHERE address=? LIMIT 1";
  rc = sqlite3_prepare_v2(db, query, -1, &stmt, 0);

  if (rc != SQLITE_OK) {
    fprintf(stderr, "%s -- Failed to create prepared statement: %s\n", __func__, sqlite3_errmsg(db));
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

cJSON* get_unspents_by_username(sqlite3* db, const char* username) {
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
    fprintf(stderr, "%s -- Failed to create prepared statement: %s\n", __func__, sqlite3_errmsg(db));
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