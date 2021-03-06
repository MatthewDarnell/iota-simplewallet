//
// Created by matth on 2/27/2020.
//

#include <sqlite3.h>
#include <sodium.h>
#include <pthread.h>
#include "../../iota-simplewallet.h"
#include "../../iota/api.h"
#include "../sqlite3/db.h"
#include "../sqlite3/stores/address.h"
#include "generate_address.h"
static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

int generate_address(const char* username, const char* seed) {
  pthread_mutex_lock(&mutex);
  sqlite3* db = get_db_handle();

  //get latest key index
  int32_t latest_offset = get_latest_offset(db, username);
  if(latest_offset < 0) {
    close_db_handle(db);
    pthread_mutex_unlock(&mutex);
    log_wallet_error("Could not get latest offset", "");
    return -1;
  } else if(latest_offset > 0) {  //increment so we'll be creating the next offset, but if it's 0 we'll start with that
    latest_offset++;
  }
  int32_t num_unused_addresses = get_num_fresh_addresses(db, username);
  int32_t num_unused_change_addresses = get_num_change_addresses(db, username);

  if(num_unused_addresses < 0 || num_unused_change_addresses < 0) {
    close_db_handle(db);
    pthread_mutex_unlock(&mutex);
    log_wallet_error("Could not get number of fresh addresses for user <%s>", username);
    return -1;
  }


  char* min_addresses = get_config("minAddressPool");
  if(!min_addresses) {
    close_db_handle(db);
    pthread_mutex_unlock(&mutex);
    log_wallet_error("Could not get config minAddressPool user <%s>", username);
    return -1;
  }

  int32_t min_address_pool = strtoul(min_addresses, NULL, 10);
  free(min_addresses);

  int32_t num_addresses_to_create = min_address_pool - num_unused_addresses;
  int32_t num_change_addresses_to_create = min_address_pool - num_unused_change_addresses;

  if(num_addresses_to_create <= 0 && num_change_addresses_to_create <= 0) {
    close_db_handle(db);
    pthread_mutex_unlock(&mutex);
    log_wallet_debug("Have sufficient amount of fresh addresses. (%d) (minimum to have is %d)", num_unused_addresses, min_address_pool);
    return 0;
  } else {
    cJSON* address = NULL;
    int i = 0;

    if(num_addresses_to_create > 0) {
      log_wallet_debug("Have insufficient amount of fresh deposit addresses. (%d) Creating (%d) more.", num_unused_addresses, num_addresses_to_create);

      cJSON* new_addresses = generate_new_addresses(seed, latest_offset, num_addresses_to_create + latest_offset);
      if(!new_addresses) {
        close_db_handle(db);
        pthread_mutex_unlock(&mutex);
        log_wallet_error("Failed to create addresses!", "")
        return -1;
      }
      cJSON_ArrayForEach(address, new_addresses) {
        const char* addr = cJSON_GetObjectItem(address, "address")->valuestring;
        uint32_t index = cJSON_GetObjectItem(address, "index")->valueint;
        if(create_address(db, addr, index, username) < 0) {
          log_wallet_error("Error storing address %s %d in database!", addr, index);
        } else {
          if(i >= num_addresses_to_create) {
            mark_address_is_change_address(db, addr);
          }
        }
        i++;
      }
      cJSON_Delete(new_addresses);
    }


    if(num_change_addresses_to_create > 0) {
      log_wallet_debug("Have insufficient amount of fresh change addresses. (%d) Creating (%d) more.", num_unused_change_addresses, num_change_addresses_to_create);
      latest_offset = get_latest_offset(db, username);
      latest_offset++;
      cJSON* new_change_addresses = generate_new_addresses(seed, latest_offset, num_change_addresses_to_create + latest_offset);
      if(!new_change_addresses) {
        close_db_handle(db);
        pthread_mutex_unlock(&mutex);
        log_wallet_error("Failed to create change addresses!", "")
        return -1;
      }

      i = 0;

      cJSON_ArrayForEach(address, new_change_addresses) {
        const char* addr = cJSON_GetObjectItem(address, "address")->valuestring;
        uint32_t index = cJSON_GetObjectItem(address, "index")->valueint;
        if(create_address(db, addr, index, username) < 0) {
          log_wallet_error("Error storing address %s %d in database!", addr, index);
        } else {
          mark_address_is_change_address(db, addr);
        }
      }
      cJSON_Delete(new_change_addresses);

    }

  }

  close_db_handle(db);
  pthread_mutex_unlock(&mutex);
  return 0;
}


int _generate_num_addresses(const char* username, char* password, int num_addrs) {
  if(num_addrs < 1) {
#ifdef WIN32
    log_wallet_error("%s: Too Few Addresses To Created .(%I64u)", __func__, num_addrs);
#else
    log_wallet_error("%s: Too Few Addresses To Created .(%llu)", __func__, num_addrs);
#endif
    return -1;
  }
  if(_verify_login(username, password, 0, 0) < 0 ) {
    log_wallet_error("%s: Error Generating %d", __func__, num_addrs);
    return -1;
  }


  char seed[128] = { 0 };
  if(decrypt_seed(seed, 127, username, password) < 0 ) {
    log_wallet_error("%s: Error Decrypting Seed", __func__);
    return -1;
  }


  pthread_mutex_lock(&mutex);
  sqlite3* db = get_db_handle();

  //get latest key index
  int32_t latest_offset = get_latest_offset(db, username);
  if(latest_offset < 0) {
    sodium_memzero(seed, 127);
    close_db_handle(db);
    pthread_mutex_unlock(&mutex);
    log_wallet_error("%s: Could not get latest offset", __func__);
    return -1;
  } else if(latest_offset > 0) {  //increment so we'll be creating the next offset, but if it's 0 we'll start with that
    latest_offset++;
  }

#ifdef WIN32
    log_wallet_debug("%s: Creating (%I64u) Addresses.", __func__, num_addrs);
#else
  log_wallet_debug("%s: Creating (%llu) Addresses.", __func__, num_addrs);
#endif

  cJSON* address = NULL;
  cJSON* new_addresses = generate_new_addresses(seed, latest_offset, num_addrs + latest_offset);

  sodium_memzero(seed, 128);

  if(!new_addresses) {
    close_db_handle(db);
    pthread_mutex_unlock(&mutex);
    log_wallet_error("%s: Failed to create addresses!", __func__);
    return -1;
  }
  cJSON_ArrayForEach(address, new_addresses) {
    const char* addr = cJSON_GetObjectItem(address, "address")->valuestring;
    uint32_t index = cJSON_GetObjectItem(address, "index")->valueint;
    if(create_address(db, addr, index, username) < 0) {
      log_wallet_error("%s: Error storing address %s %d in database!", __func__< addr, index);
    }
  }
  cJSON_Delete(new_addresses);
  close_db_handle(db);
  pthread_mutex_unlock(&mutex);
  return 0;
}