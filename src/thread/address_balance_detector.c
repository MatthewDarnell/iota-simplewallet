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
#include "../database/sqlite3/stores/address.h"
#include "../iota/api.h"
#include "../database/sqlite3/stores/account.h"
#include "event_queue.h"
#include "../iota-simplewallet.h"

static cJSON* accounts = NULL;

void thread_address_balance_detector(void* args) {

  log_wallet_info("Starting Address Balance Detector Thread", "");
  int* quit_flag = (int*)args;

  sqlite3* db = get_db_handle();

  accounts = get_all_accounts(db);

  while(1) {
    if(*quit_flag != 0) {
      break;
    }
    Sleep(5 * 1000);


    //See if any balance_changed events need to be fired
    cJSON* acc = NULL;
    cJSON_ArrayForEach(acc, accounts) {
      char* username = cJSON_GetObjectItem(acc, "username")->valuestring;
      char* balance = cJSON_GetObjectItem(acc, "balance")->valuestring;
      cJSON* a = get_account_by_username(db, username);
      if(!a) {
        continue;
      }
      char* b = cJSON_GetObjectItem(a, "balance")->valuestring;
      if(strcasecmp(balance, b) != 0) { //balance has updated since app launch
        cJSON_DeleteItemFromObject(a, "serial");
        cJSON_DeleteItemFromObject(a, "seed_ciphertext");
        cJSON_DeleteItemFromObject(a, "salt");
        cJSON_DeleteItemFromObject(a, "nonce");
        cJSON_DeleteItemFromObject(a, "is_synced");
        cJSON_DeleteItemFromObject(a, "created_at");
        char* string = cJSON_PrintUnformatted(a);
        push_new_event("balance_changed", string);
        free(string);
        cJSON_DeleteItemFromObject(acc, "balance");
        cJSON_AddStringToObject(acc, "balance", b);
      }
      cJSON_Delete(a);
    }

    int num_known_accounts = cJSON_GetArraySize(accounts);
    cJSON* current_accounts = get_all_accounts(db);
    int num_current_known_accounts = cJSON_GetArraySize(current_accounts);

    if(num_known_accounts != num_current_known_accounts) {  //An account was created or deleted
      cJSON_ArrayForEach(acc, current_accounts) {
        cJSON_DeleteItemFromObject(acc, "serial");
        cJSON_DeleteItemFromObject(acc, "seed_ciphertext");
        cJSON_DeleteItemFromObject(acc, "salt");
        cJSON_DeleteItemFromObject(acc, "nonce");
        cJSON_DeleteItemFromObject(acc, "is_synced");
        cJSON_DeleteItemFromObject(acc, "created_at");
        char* string = cJSON_PrintUnformatted(acc);
        push_new_event("balance_changed", string);
        free(string);
      }
      cJSON_Delete(accounts);
      accounts = cJSON_Duplicate(current_accounts, 1);
    }
    cJSON_Delete(current_accounts);
    //End balance_changed events



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
            if(set_address_balance(db, address, balance) < 0) {
              log_wallet_error("%s unable to set address balance for %s %s", __func__, address, balance);
            }
          }
        }
      }
      cJSON_Delete(all_address_array);
    }
  }
  if(accounts) {
    cJSON_Delete(accounts);
  }
  log_wallet_info("Shutting Down Address Balance Detector Thread", "");
  close_db_handle(db);
  pthread_exit(0);
}