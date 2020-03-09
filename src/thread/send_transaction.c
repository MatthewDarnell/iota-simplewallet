//
// Created by matth on 3/3/2020.
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
#include "../database/sqlite3/stores/incoming_transaction.h"
#include "../database/sqlite3/stores/outgoing_transaction.h"
#include "../database/helpers/store_iota_inputs.h"
#include "../iota/api.h"
#include "send_transaction.h"

void thread_send_transaction(void* args) {
  log_wallet_info("Starting Send Transaction Thread", "");
  int* quit_flag = (int*)args;

  sqlite3* db = get_db_handle();

  while(1) {
    if(*quit_flag != 0) {
      break;
    }
    Sleep(5 * 1000);

    cJSON *transactions_to_send_array = get_all_unsent_outgoing_transactions(db);
    if (!transactions_to_send_array) {
      continue;
    } else if(cJSON_GetArraySize(transactions_to_send_array) < 1) {
      continue;
    }

    cJSON* tx = NULL;

    char bundle[128] = { 0 };
    char hash[128] = { 0 };

    cJSON_ArrayForEach(tx, transactions_to_send_array) {
      uint64_t serial = cJSON_GetObjectItem(tx, "serial")->valueint;
      const char* trytes = cJSON_GetObjectItem(tx, "trytes")->valuestring;
     /* if(send_trytes(bundle, 127, hash, 127, serial, trytes) == 0) {
        mark_outgoing_transaction_sent(db, serial, bundle, hash);
      }*/
    }

  }
  log_wallet_info("Shutting Send Transaction Detector Thread", "");
  close_db_handle(db);
  pthread_exit(0);

}
