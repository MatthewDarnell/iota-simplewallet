//
// Created by Matthew Darnell on 3/9/20.
//

#ifdef WIN32
#include <windows.h>
#else
#include <unistd.h>
#define Sleep(x) sleep(x/1000)
#endif

#include <stdio.h>
#include <pthread.h>
#include <cjson/cJSON.h>
#include "../database/sqlite3/db.h"
#include "../config/logger.h"
#include "../database/sqlite3/stores/address.h"
#include "../iota/api.h"

void thread_spent_from_detector(void* args) {
  log_wallet_info("Starting Address Spent From Detector Thread", "");
  int* quit_flag = (int*)args;

  sqlite3* db = get_db_handle();

  while(1) {
    if (*quit_flag != 0) {
      break;
    }
    Sleep(5 * 1000);

    //Update any addresses which have been spent from
    cJSON *json_address = NULL;
    cJSON *unspents = get_unspent_addresses(db);
    if (unspents) {
      were_addresses_spent_from(&unspents);
      cJSON_ArrayForEach(json_address, unspents) {
        int spent = cJSON_GetObjectItem(json_address, "spent_from")->valueint;
        if (spent > 0) {
          char *addr = cJSON_GetObjectItem(json_address, "address")->valuestring;
          mark_address_spent_from(db, addr);
        }
      }
      cJSON_Delete(unspents);
    }
  }
  log_wallet_info("Shutting Down Address Spent From Detector Thread", "");
  close_db_handle(db);
  pthread_exit(0);
}