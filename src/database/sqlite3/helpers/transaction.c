//
// Created by matth on 2/28/2020.
//

#include <cjson/cJSON.h>
#include "../stores/incoming_transaction.h"
#include "transaction.h"

char* get_incoming_transaction_by_hash(sqlite3* db, char* hash) {
  cJSON* json = get_incoming_transaction_hash(db, hash);
  if(!json) {
    return NULL;
  }
  char* ret_val = cJSON_Print(json);
  cJSON_Delete(json);
  return ret_val;
}

char* get_incoming_transactions(sqlite3* db, char* username, int offset, int num) {
  cJSON* json = get_all_incoming_transactions(db, username, offset, num);
  if(!json) {
    return NULL;
  }
  char* ret_val = cJSON_Print(json);
  cJSON_Delete(json);
  return ret_val;
}