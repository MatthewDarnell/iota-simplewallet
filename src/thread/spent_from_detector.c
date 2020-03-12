//
// Created by Matthew Darnell on 3/9/20.
//

#ifdef WIN32
#include <windows.h>
#else
#include <unistd.h>
#define Sleep(x) sleep(x/1000)
#endif

#include <pthread.h>
#include <cjson/cJSON.h>
#include "../database/sqlite3/db.h"
#include "../iota-simplewallet.h"
#include "../database/sqlite3/stores/address.h"
#include "../iota/api.h"

void thread_spent_from_detector(void* args) {
  log_wallet_info("Starting Address Spent From Detector Thread", "");
  int* quit_flag = (int*)args;

  sqlite3* db = get_db_handle();

  int i;
  while(1) {
    for(i=0; i < 50; i++) {
      Sleep(100);
      if(*quit_flag != 0) {
        break;
      }
    }
    if(*quit_flag != 0) {
      break;
    }

    //Update any addresses which have been spent from
    cJSON *json_address = NULL;
    cJSON *unspents = get_unspent_addresses(db);
    if (unspents) {
      if(cJSON_GetArraySize(unspents) > 0) {
        if(were_addresses_spent_from(&unspents) == 0) {
          cJSON_ArrayForEach(json_address, unspents) {
            int spent = cJSON_GetObjectItem(json_address, "spent_from")->valueint;
            if (spent > 0) {
              char *addr = cJSON_GetObjectItem(json_address, "address")->valuestring;
              if(mark_address_spent_from(db, addr) < 0) {
                log_wallet_error("%s unable to mark address spent from -- %s", __func__, addr);
              }
            }
          }
        }
      }
      cJSON_Delete(unspents);
    }
  }
  log_wallet_info("Shutting Down Address Spent From Detector Thread", "");
  close_db_handle(db);
  pthread_exit(0);
}