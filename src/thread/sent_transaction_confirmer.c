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
#include "../config/logger.h"
#include "../database/sqlite3/stores/outgoing_transaction.h"
#include "../iota/api.h"


void thread_sent_transaction_confirmer(void* args) {
  log_wallet_info("Starting Sent Transaction Confirmer Thread", "");
  int* quit_flag = (int*)args;

  sqlite3* db = get_db_handle();

  while(1) {
    if (*quit_flag != 0) {
      break;
    }
    Sleep(5 * 1000);

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
      }
    }






    cJSON_Delete(unconfirmed_outgoing_transactions);
  }
  log_wallet_info("Shutting Down Sent Transaction Confirmer Thread", "");
  close_db_handle(db);
  pthread_exit(0);
}