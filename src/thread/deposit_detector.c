//
// Created by matth on 2/26/2020.
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
#include "../database/sqlite3/stores/incoming_transaction.h"
#include "../database/helpers/store_iota_inputs.h"
#include "../iota/api.h"

void thread_deposit_detector(void* args) {
  log_wallet_info("Starting Deposit Detector Thread", "");
  int* quit_flag = (int*)args;

  sqlite3* db = get_db_handle();

  while(1) {
    if(*quit_flag != 0) {
      break;
    }
    Sleep(5 * 1000);

    cJSON *address_array = get_deposit_addresses(db);
    if (!address_array) {
      continue;
    } else if(cJSON_GetArraySize(address_array) < 1) {
      cJSON_Delete(address_array);
      continue;
    }

    int ret_val = 0;
    get_address_balance(&address_array, 1, 1);

    if(cJSON_GetArraySize(address_array) < 1) {
      cJSON_Delete(address_array);
      continue;
    }

    cJSON* addr_obj = NULL;

    //Found at least 1 address with a balance. Update our balances in db
    cJSON_ArrayForEach(addr_obj, address_array) {
      const char* address = cJSON_GetObjectItem(addr_obj, "address")->valuestring;
      const char* balance = cJSON_GetObjectItem(addr_obj, "balance")->valuestring;
      set_address_balance(db, address, balance);
    }

    int input_len = cJSON_GetArraySize(address_array);

    if(input_len < 1) {
      continue;
    }

    get_transaction_inputs_to_address(&address_array);

    input_len = cJSON_GetArraySize(address_array);

    if(input_len < 1) { //No transactions found
      continue;
    }


    cJSON* obj, *transaction;
    cJSON_ArrayForEach(obj, address_array) {
      cJSON* transaction_array = cJSON_GetObjectItem(obj, "transactions");
      int j = 0;
      cJSON_ArrayForEach(transaction, transaction_array) {
        char* hash = cJSON_GetObjectItem(transaction, "hash")->valuestring;
        cJSON* tx = get_incoming_transaction_hash(db, hash);
        if(tx) {
          log_wallet_debug("Removing known transaction: %s", hash);
          cJSON_DeleteItemFromArray(transaction_array, j);
          j--;
        }
        j++;
      }
    }

    get_latest_inclusion(&address_array, false);
    char* array_str = cJSON_PrintUnformatted(address_array);
    cJSON_Delete(address_array);

    if((ret_val = store_inputs(db, array_str)) < 0) {
      log_wallet_error("Deposit Detector failed to store tx inputs (%d)", ret_val);
    }
    free(array_str);
  }
  log_wallet_info("Shutting Down Deposit Detector Thread", "");
  close_db_handle(db);
  pthread_exit(0);
}
