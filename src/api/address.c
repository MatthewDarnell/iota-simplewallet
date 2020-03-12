//
// Created by matth on 2/28/2020.
//

#include <stdlib.h>
#include <sqlite3.h>
#include <cjson/cJSON.h>
#include "../database/sqlite3/db.h"
#include "../database/sqlite3/stores/address.h"
#include "../iota-simplewallet.h"

char* get_new_address(char* username) {
  sqlite3* db = get_db_handle();
  cJSON* json = get_next_fresh_address(db, username);
  close_db_handle(db);
  if(!json) {
    return NULL;
  } else {
    char *ret_val = cJSON_PrintUnformatted(json);
    cJSON_Delete(json);
    return ret_val;
  }
}

