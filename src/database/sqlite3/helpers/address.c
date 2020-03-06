//
// Created by matth on 2/28/2020.
//

#include <stdlib.h>
#include <cjson/cJSON.h>
#include "../../../config/config.h"
#include "../../../config/logger.h"
#include "../stores/address.h"
#include "../db.h"
#include "address.h"

char* get_new_address(char* username) {
  sqlite3* db = get_db_handle();
  cJSON* json = NULL;
  if(username) {
    json = get_next_fresh_address(db, username);
  } else {
    char* u = get_config("mainAccount");
    if(!u) {
      log_wallet_error("%s Unable to get mainAccount\n", __func__);
      return NULL;
    }
    json = get_next_fresh_address(db, u);
    free(u);
  }

  close_db_handle(db);
  if(!json) {
    return NULL;
  } else {
    char *ret_val = cJSON_PrintUnformatted(json);
    cJSON_Delete(json);
    return ret_val;
  }
}

