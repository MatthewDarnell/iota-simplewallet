//
// Created by Matthew Darnell on 3/6/20.
//

#include <sqlite3.h>
#include <pthread.h>
#include "../../config/config.h"
#include "../../config/logger.h"
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

  cJSON* inputs = get_inputs(seed);
  cJSON* input = NULL;

  uint64_t latest_key_index = 0;
  cJSON_ArrayForEach(input, inputs) {
    uint64_t key_index = strtoull(cJSON_GetObjectItem(input, "keyIndex")->valuestring, NULL, 10);
    if(latest_key_index <= key_index) {
      latest_key_index = key_index;
    }
  }


  uint8_t input_lut[latest_key_index];
  memset(input_lut, 0, latest_key_index);

  cJSON_ArrayForEach(input, inputs) {
    uint64_t key_index = strtoull(cJSON_GetObjectItem(input, "keyIndex")->valuestring, NULL, 10);
    input_lut[key_index] = 1;
  }


  cJSON* new_addresses = generate_new_addresses(seed, 0, latest_key_index);
  if(!new_addresses) {
    pthread_mutex_unlock(&mutex);
    log_wallet_error("Failed to sync account!", "")
    return -1;
  }


  cJSON* address = NULL;

  int i = 0;
  cJSON_ArrayForEach(address, new_addresses) {
    const char* addr = cJSON_GetObjectItem(address, "address")->valuestring;
    uint32_t index = cJSON_GetObjectItem(address, "index")->valueint;
    if(create_address(db, addr, index, username) < 0) {
      log_wallet_error("Error storing address %s %d in database!", addr, index);
    }
    if(input_lut[i] == 1) {
      log_wallet_info("Adding address %s @ key index %d\n", addr, i);
      cJSON_ArrayForEach(input, inputs) {
        uint64_t key_index = strtoull(cJSON_GetObjectItem(input, "keyIndex")->valuestring, NULL, 10);
        if(index == key_index) {
          char* balance = cJSON_GetObjectItem(input, "balance")->valuestring;
          set_address_balance(db, addr, balance);
          break;
        }
      }
    }
    i++;
  }

  cJSON_Delete(new_addresses);

  //Get Inputs does not always seem to find all inputs. After getinputs is called, we are going
  //to manually check address balance for the first <minAddressesToCheckWhenSyncing> addresses,
  //as long as <minAddressesToCheckWhenSyncing> is greater than the highest key index found by getinputs
  char* str_minAddressesToCheckWhenSyncing = get_config("minAddressesToCheckWhenSyncing");
  if(str_minAddressesToCheckWhenSyncing) {
    int minAddressesToCheckWhenSyncing = strtol(str_minAddressesToCheckWhenSyncing, NULL, 10);
    if(minAddressesToCheckWhenSyncing > latest_key_index) {
      cJSON* addresses = generate_new_addresses(seed, 0, minAddressesToCheckWhenSyncing);
      if(addresses) {
        cJSON_ArrayForEach(address, addresses) {
          const char* addr = cJSON_GetObjectItem(address, "address")->valuestring;
          uint32_t index = cJSON_GetObjectItem(address, "index")->valueint;
          create_address(db, addr, index, username);  //will fail on duplicate addresses, just ignore
        }
        get_address_balance(&addresses, 1);
        cJSON_ArrayForEach(address, addresses) {
          const char* addr = cJSON_GetObjectItem(address, "address")->valuestring;
          const char* balance = cJSON_GetObjectItem(address, "balance")->valuestring;
          set_address_balance(db, addr, balance);
        }
        cJSON_Delete(address);
      }
    }
    free(str_minAddressesToCheckWhenSyncing);
  }


  //Find if any addresses have been spent
  cJSON* unspents = get_unspent_addresses_by_username(db, username);
  were_addresses_spent_from(&unspents);
  cJSON_ArrayForEach(address, unspents) {
    int spent = cJSON_GetObjectItem(address, "spent_from")->valueint;
    char* addr = cJSON_GetObjectItem(address, "address")->valuestring;
    if(spent > 0) {
      mark_address_spent_from(db, addr);
    }
  }



  mark_account_synced(db, username);
  close_db_handle(db);
  pthread_mutex_unlock(&mutex);
  return 0;
}