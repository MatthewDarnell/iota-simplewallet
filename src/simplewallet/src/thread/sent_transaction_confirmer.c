//
// Created by Matthew Darnell on 3/11/20.
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
#include "../database/sqlite3/stores/outgoing_transaction.h"
#include "../iota/api.h"
#include "event_queue.h"


void thread_sent_transaction_confirmer(void* args) {
  log_wallet_info("Starting Sent Transaction Confirmer Thread", "");
  int* quit_flag = (int*)args;

  sqlite3* db = get_db_handle();

  int i;
  while(1) {
    for(i=0; i < 10; i++) {
      Sleep(1000);
      if(*quit_flag != 0) {
        break;
      }
    }
    if(*quit_flag != 0) {
      break;
    }

    cJSON* unconfirmed_outgoing_transactions = get_all_unconfirmed_outgoing_transactions(db);

    if(!unconfirmed_outgoing_transactions) {
      continue;
    }
    if(cJSON_GetArraySize(unconfirmed_outgoing_transactions) < 1) {
      cJSON_Delete(unconfirmed_outgoing_transactions);
      continue;
    }
    get_latest_inclusion_by_tx_hash(&unconfirmed_outgoing_transactions);
    cJSON* tx = NULL;

    cJSON_ArrayForEach(tx, unconfirmed_outgoing_transactions) {
      int confirmed = cJSON_GetObjectItem(tx, "confirmed")->valueint;
      if(confirmed > 0) {
        int serial = cJSON_GetObjectItem(tx, "serial")->valueint;
        char* hash = cJSON_GetObjectItem(tx, "hash")->valuestring;
        char* bundle = cJSON_GetObjectItem(tx, "bundle")->valuestring;
        mark_outgoing_transaction_confirmed(db, serial, (const char*)bundle, (const char*)hash);
        cJSON_DeleteItemFromObject(tx, "trytes");

        char* dest_address = cJSON_GetObjectItem(tx, "dest_address")->valuestring;
        cJSON_AddStringToObject(tx, "address", dest_address);

        char* string = cJSON_PrintUnformatted(tx);
        push_new_event("sent_transaction_confirmed", string);
        free(string);
      }
    }
    cJSON_Delete(unconfirmed_outgoing_transactions);
  }
  log_wallet_info("Shutting Down Sent Transaction Confirmer Thread", "");
  close_db_handle(db);
  pthread_exit(0);
}