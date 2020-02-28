//
// Created by matth on 2/28/2020.
//

#include <cjson/cJSON.h>
#include "../stores/address.h"
#include "address.h"

char* get_new_address(sqlite3* db, char* username) {
  cJSON* json = get_next_fresh_address(db, username);
  if(!json) {
    return NULL;
  } else {
    char *ret_val = cJSON_Print(json);
    cJSON_Delete(json);
    return ret_val;
  }
}

