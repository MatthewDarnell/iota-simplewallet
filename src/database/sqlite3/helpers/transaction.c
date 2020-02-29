//
// Created by matth on 2/28/2020.
//

#include <cjson/cJSON.h>
#include "../stores/incoming_transaction.h"
#include "../db.h"
#include "transaction.h"

char* get_incoming_transaction_by_hash(char* hash) {
  sqlite3* db = get_db_handle();
  cJSON* json = get_incoming_transaction_hash(db, hash);
  close_db_handle(db);
  if(!json) {
    return NULL;
  }
  char* ret_val = cJSON_Print(json);
  cJSON_Delete(json);
  return ret_val;
}

char* get_incoming_transactions(char* username, int offset, int num) {
  sqlite3* db = get_db_handle();
  cJSON* json = get_all_incoming_transactions(db, username, offset, num);
  close_db_handle(db);
  if(!json) {
    return NULL;
  }
  char* ret_val = cJSON_Print(json);
  cJSON_Delete(json);
  return ret_val;
}