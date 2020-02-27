//
// Created by matth on 2/27/2020.
//

#include <sqlite3.h>
#include <pthread.h>
#include "../../../config/config.h"
#include "../../../config/logger.h"
#include "../../../iota/api.h"
#include "../db.h"
#include "../stores/address.h"
#include "generate_address.h"
static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

int generate_address(const char* username, const char* seed) {
  pthread_mutex_lock(&mutex);
  sqlite3* db = get_db_handle();

  //get latest key index
  int32_t latest_offset = get_latest_offset(db, username);
  if(latest_offset < 0) {
    pthread_mutex_unlock(&mutex);
    log_wallet_error("Could not get latest offset", "");
    return -1;
  } else if(latest_offset > 0) {  //increment so we'll be creating the next offset, but if it's 0 we'll start with that
    latest_offset++;
  }
  int32_t num_unused_addresses = get_num_fresh_addresses(db, username);

  if(num_unused_addresses < 0) {
    pthread_mutex_unlock(&mutex);
    log_wallet_error("Could not get number of fresh addresses for user <%s>", username);
    return -1;
  }


  char* min_addresses = get_config("minAddressPool");
  if(!min_addresses) {
    pthread_mutex_unlock(&mutex);
    log_wallet_error("Could not get config minAddressPool user <%s>", username);
    return -1;
  }

  int32_t min_address_pool = strtoul(min_addresses, NULL, 10);
  free(min_addresses);

  int32_t num_addresses_to_create = min_address_pool - num_unused_addresses;

  if(num_addresses_to_create <= 0) {
    pthread_mutex_unlock(&mutex);
    log_wallet_debug("Have sufficient amount of fresh addresses. (%d) (minimum to have is %d)", num_unused_addresses, min_address_pool);
    return 0;
  } else {
    log_wallet_debug("Have insufficient amount of fresh addresses. (%d) Creating (%d) more.", num_unused_addresses, num_addresses_to_create);
  }



  cJSON* new_addresses = get_new_address(seed, latest_offset, num_addresses_to_create + latest_offset);
  if(!new_addresses) {
    pthread_mutex_unlock(&mutex);
    log_wallet_error("Failed to create addresses!", "")
    return -1;
  }

  cJSON* address = NULL;

  cJSON_ArrayForEach(address, new_addresses) {
    const char* addr = cJSON_GetObjectItem(address, "address")->valuestring;
    uint32_t index = cJSON_GetObjectItem(address, "index")->valueint;
    if(create_address(db, addr, index, username) < 0) {
      log_wallet_error("Error storing address %s %d in database!", addr, index);
    }
  }
  cJSON_Delete(new_addresses);
  close_db_handle(db);
  pthread_mutex_unlock(&mutex);
  return 0;
}