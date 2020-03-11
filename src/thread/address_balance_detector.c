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
#include "../config/logger.h"
#include "../database/sqlite3/stores/address.h"
#include "../iota/api.h"

void thread_address_balance_detector(void* args) {

  log_wallet_info("Starting Address Balance Detector Thread", "");
  int* quit_flag = (int*)args;

  sqlite3* db = get_db_handle();

  while(1) {
    if(*quit_flag != 0) {
      break;
    }
    Sleep(5 * 1000);

    cJSON* addr_obj = NULL;

    //Get and update all address balances
    cJSON *all_address_array = get_all_addresses(db);
    if(all_address_array) {
      if(cJSON_GetArraySize(all_address_array) > 0) {
        get_address_balance(&all_address_array, 0, 0);
        if(cJSON_GetArraySize(all_address_array) > 0) {
          cJSON_ArrayForEach(addr_obj, all_address_array) {
            const char* address = cJSON_GetObjectItem(addr_obj, "address")->valuestring;
            const char* balance = cJSON_GetObjectItem(addr_obj, "balance")->valuestring;
            set_address_balance(db, address, balance);
          }
        }
      }
      cJSON_Delete(all_address_array);
    }
  }
  log_wallet_info("Shutting Down Address Balance Detector Thread", "");
  log_wallet_info("closing down...", "");
  close_db_handle(db);
  pthread_exit(0);
}