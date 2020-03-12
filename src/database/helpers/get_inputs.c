//
// Created by Matthew Darnell on 3/6/20.
//

#include <sqlite3.h>
#include <pthread.h>
#include "../../iota-simplewallet.h"
#include "../../iota/api.h"
#include "../sqlite3/db.h"
#include "../sqlite3/stores/account.h"
#include "../sqlite3/stores/address.h"

static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
int get_account_inputs(const char* username, const char* seed) {
  pthread_mutex_lock(&mutex);
  sqlite3* db = get_db_handle();

  int is_synced = is_account_synced(db, username);
  if(is_synced) {
    pthread_mutex_unlock(&mutex);
    return 0;
  }

  //Get Inputs does not always seem to find all inputs.
  //Manually check address balance for the first <minAddressesToCheckWhenSyncing> addresses,
  //as long as <minAddressesToCheckWhenSyncing> is greater than the highest key index found by getinputs
  char* str_minAddressesToCheckWhenSyncing = get_config("minAddressesToCheckWhenSyncing");
  if(!str_minAddressesToCheckWhenSyncing) {
    log_wallet_error("%s cannot find minAddressesToCheckWhenSyncing", __func__);
    pthread_mutex_unlock(&mutex);
    return 0;
  }

  uint64_t minAddressesToCheckWhenSyncing = strtoull(str_minAddressesToCheckWhenSyncing, NULL, 10);
  uint64_t nextAddressesToCheck = minAddressesToCheckWhenSyncing;
  free(str_minAddressesToCheckWhenSyncing);

  uint64_t start_index = 0;
  int found_balance = 0;
  uint64_t temp_balance = 0;
  cJSON* address = NULL;

  while(1) {
    found_balance = 0;
#ifdef WIN32
    log_wallet_debug("Syncing Account. Checking addresses %I64u through %I64u\n", start_index, minAddressesToCheckWhenSyncing)
#else
    log_wallet_debug("Syncing Account. Checking addresses %llu through %llu\n", start_index, nextAddressesToCheck)
#endif
    cJSON* addresses = generate_new_addresses(seed, start_index, nextAddressesToCheck);
    if(!addresses) {
#ifdef WIN32
      log_wallet_error("Could not find addresses starting at index.(%I64u)", start_index);
#else
      log_wallet_error("Could not find addresses starting at index.(%llu)", start_index);
#endif
      break;
    }

    get_address_balance(&addresses, 1, 0);

    cJSON_ArrayForEach(address, addresses) {
      if(!cJSON_HasObjectItem(address, "balance")) {
        continue;
      }
      temp_balance = strtoull(cJSON_GetObjectItem(address, "balance")->valuestring, NULL, 10);
      if(temp_balance > 0) {  //At least 1 address in this batch has a non-zero balance
        log_wallet_debug("Found an address with a balance of %lld\n", temp_balance);
        start_index = nextAddressesToCheck;
        nextAddressesToCheck += minAddressesToCheckWhenSyncing;
        found_balance = 1;
        break;
      }
    }

    if(found_balance == 0) {  //No more addresses found with a balance in the last <minAddressesToCheckWhenSyncing> checked. Let's break out
      break;
    }


    cJSON_ArrayForEach(address, addresses) {  //At least one of these addresses has a balance. Let's save these addresses
      const char* addr = cJSON_GetObjectItem(address, "address")->valuestring;
      uint32_t index = cJSON_GetObjectItem(address, "index")->valueint;
      create_address(db, addr, index, username);  //will fail on duplicate addresses, just ignore
      if(cJSON_HasObjectItem(address, "balance")) {
        char* balance = cJSON_GetObjectItem(address, "balance")->valuestring;
        temp_balance = strtoull(balance, NULL, 10);
        if(temp_balance > 0) {
          if(set_address_balance(db, addr, balance) < 0) {
            log_wallet_error("%s unable to set address balance for %s %s", __func__, addr, balance);
          }
        }
      }
    }

    cJSON_Delete(addresses);

  }


  //Find if any addresses have been spent
  cJSON* unspents = get_unspent_addresses_by_username(db, username);
  if(0 != were_addresses_spent_from(&unspents)) {
    close_db_handle(db);
    pthread_mutex_unlock(&mutex);
    return 0;
  }

  cJSON_ArrayForEach(address, unspents) {
    int spent = cJSON_GetObjectItem(address, "spent_from")->valueint;
    char* addr = cJSON_GetObjectItem(address, "address")->valuestring;
    if(spent > 0) {
      if(mark_address_spent_from(db, addr) < 0) {
        log_wallet_error("%s unable to mark address spent from -- %s", __func__, addr);
      }
    }
  }



  mark_account_synced(db, username);
  close_db_handle(db);
  pthread_mutex_unlock(&mutex);
  return 0;
}