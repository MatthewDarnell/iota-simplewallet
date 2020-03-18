//
// Created by matth on 3/3/2020.
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
#include "../database/sqlite3/stores/address.h"
#include "../database/sqlite3/stores/outgoing_transaction.h"
#include "../iota/api.h"
#include "../iota-simplewallet.h"
#include "event_queue.h"

void thread_send_transaction(void* args) {
  log_wallet_info("Starting Send Transaction Thread", "");
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

    cJSON *transactions_to_send_array = get_all_unsent_outgoing_transactions(db);
    if (!transactions_to_send_array) {
      continue;
    } else if(cJSON_GetArraySize(transactions_to_send_array) < 1) {
      cJSON_Delete(transactions_to_send_array);
      continue;
    }

    cJSON* tx = NULL;

    char bundle[128] = { 0 };
    char hash[128] = { 0 };

    cJSON_ArrayForEach(tx, transactions_to_send_array) {
      uint64_t serial = cJSON_GetObjectItem(tx, "serial")->valueint;
      const char* trytes = cJSON_GetObjectItem(tx, "trytes")->valuestring;
      const char* dest_address = cJSON_GetObjectItem(tx, "dest_address")->valuestring;
      if(send_trytes(bundle, 127, hash, 127, serial, trytes) == 0) {
        mark_outgoing_transaction_sent(db, serial, bundle, hash);
        cJSON_DeleteItemFromObject(tx, "trytes");
        cJSON_AddStringToObject(tx, "hash", hash);
        cJSON_AddStringToObject(tx, "bundle", bundle);

        cJSON_AddStringToObject(tx, "address", dest_address);

        cJSON_DeleteItemFromObject(tx, "sent");
        cJSON_AddNumberToObject(tx, "sent", 1);

        char* string = cJSON_PrintUnformatted(tx);
        push_new_event("transaction_sent",string);
        free(string);
      }
    }
    cJSON_Delete(transactions_to_send_array);
  }
  log_wallet_info("Shutting Send Transaction Detector Thread", "");
  close_db_handle(db);
  pthread_exit(0);

}
