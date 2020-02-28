//
// Created by matth on 2/26/2020.
//

#ifdef WIN32
#include <windows.h>
void usleep(__int64 usec)
{
  HANDLE timer;
  LARGE_INTEGER ft;

  ft.QuadPart = -(10*usec); // Convert to 100 nanosecond interval, negative value indicates relative time

  timer = CreateWaitableTimer(NULL, TRUE, NULL);
  SetWaitableTimer(timer, &ft, 0, NULL, NULL, 0);
  WaitForSingleObject(timer, INFINITE);
  CloseHandle(timer);
}
#else
#include <unistd.h>
#endif

#include <stdio.h>
#include <pthread.h>
#include <cjson/cJSON.h>
#include "../database/sqlite3/db.h"
#include "../config/config.h"
#include "../config/logger.h"
#include "../database/sqlite3/stores/address.h"
#include "../database/sqlite3/helpers/store_iota_inputs.h"
#include "../iota/api.h"
#include "deposit_detector.h"

void thread_deposit_detector(void* args) {
  log_wallet_info("Starting Deposit Detector Thread", "");
  int* quit_flag = (int*)args;

  sqlite3* db = get_db_handle();
  int i = 0;

  while(1) {
    if(*quit_flag != 0) {
      break;
    }
    cJSON *address_array = get_deposit_addresses(db);

    if (!address_array) {
      continue;
    }

    int ret_val = 0;
    get_address_balance(&address_array, 1);

    if(cJSON_GetArraySize(address_array) < 1) {
      cJSON_Delete(address_array);
      continue;
    }

    //Found at least 1 address with a balance. Update our balances in db
    cJSON* addr_obj = NULL;
    cJSON_ArrayForEach(addr_obj, address_array) {
      const char* address = cJSON_GetObjectItem(addr_obj, "address")->valuestring;
      const char* balance = cJSON_GetObjectItem(addr_obj, "balance")->valuestring;
      set_address_balance(db, address, balance);
    }

    get_transaction_inputs_to_address(&address_array);
    get_latest_inclusion(&address_array, false);

    if((ret_val = store_inputs(db, address_array)) < 0) {
      log_wallet_error("Deposit Detector failed to store tx inputs (%d)", ret_val);
    }
    cJSON_Delete(address_array);
    i++;
     usleep(5 * 1000 * 1000);
  }
  log_wallet_info("Shutting Down Deposit Detector Thread", "");
  close_db_handle(db);
  pthread_exit(0);

}