//
// Created by matth on 2/28/2020.
//

#include <stdlib.h>
#include <string.h>
#include <sqlite3.h>
#include <cjson/cJSON.h>
#include "../database/sqlite3/db.h"
#include "../database/sqlite3/stores/address.h"
#include "../database/sqlite3/stores/incoming_transaction.h"
#include "../database/sqlite3/stores/outgoing_transaction.h"
#include "../iota/api.h"
#include "../iota-simplewallet.h"

char* get_incoming_transaction_by_hash(char* hash) {
  sqlite3* db = get_db_handle();
  cJSON* json = get_incoming_transaction_hash(db, hash);
  close_db_handle(db);
  if(!json) {
    return NULL;
  }
  char* ret_val = cJSON_Print(json);
  cJSON_Delete(json);
  return ret_val;
}

char* get_incoming_transactions(char* username, int offset, int num) {
  sqlite3* db = get_db_handle();
  cJSON* json = NULL;
  if(username) {
    json = get_all_incoming_transactions(db, username, offset, num);
  } else {
    char* u = get_config("mainAccount");
    if(!u) {
      log_wallet_error("%s Unable to get mainAccount\n", __func__);
      return NULL;
    }
    json = get_all_incoming_transactions(db, u, offset, num);
    free(u);
  }

  close_db_handle(db);
  if(!json) {
    return NULL;
  }
  char* ret_val = cJSON_Print(json);
  cJSON_Delete(json);
  return ret_val;
}

int create_transaction(char* username, char* password, char* dest_address, uint64_t value) {
  char* u = NULL;
  int free_user = 0;

  if(username) {
    u = username;
  } else {
    u = get_config("mainAccount");
    if(!u) {
      log_wallet_error("%s Unable to get mainAccount\n", __func__);
      return -1;
    }
    free_user = 1;
  }


  int verified = verify_login(u, password, 0);
  if(verified != 0) {
    log_wallet_error("Invalid Password, cannot create transaction", "");
    if(free_user == 1) {
      free(u);
    }
    return -1;
  }
  sqlite3* db = get_db_handle();

  //Get change address
  cJSON* change_address_json = get_next_change_address(db, u);
  if(!change_address_json) {
    close_db_handle(db);
    log_wallet_error("Could not get change address for spending", "");
    if(free_user == 1) {
      free(u);
    }
    return -1;
  }

  char* change_address = cJSON_GetObjectItem(change_address_json, "address")->valuestring;

  //Get minimal set of inputs to use for spending
  cJSON* inputs = get_addresses_for_spending(db, u);
  if(!inputs) {
    close_db_handle(db);
    cJSON_Delete(change_address_json);
    log_wallet_error("No Inputs for spending transaction", "");
    if(free_user == 1) {
      free(u);
    }
    return -1;
  }
  int num_inputs = cJSON_GetArraySize(
                    cJSON_GetObjectItem(inputs, "inputs")
                  );

  char* str_balance = cJSON_GetObjectItem(inputs, "total_balance")->valuestring;
  uint64_t total_balance = strtoull(str_balance, NULL, 10);

  if(total_balance < value || num_inputs < 1) {
    close_db_handle(db);
    cJSON_Delete(change_address_json);
    cJSON_Delete(inputs);
    log_wallet_error("Insufficient Funds for Creating Transaction. (Balance: %s) (Input Count: %d)\n", str_balance, num_inputs);
    if(free_user == 1) {
      free(u);
    }
    return -1;
  }

  uint64_t running_balance = 0;
  cJSON* obj = NULL;
  cJSON* inputs_to_use = cJSON_CreateArray();



  char seed[128] = { 0 };
  if(decrypt_seed(seed, 127, u, password) < 0) {
    cJSON_Delete(change_address_json);
    close_db_handle(db);
    log_wallet_error("Error creating transaction, could not decrypt seed", "")
    if(free_user == 1) {
      free(u);
    }
    return -1;
  }


  cJSON_ArrayForEach(obj, cJSON_GetObjectItem(inputs, "inputs")) {
    char* address = cJSON_GetObjectItem(obj, "address")->valuestring;
    char* balance = cJSON_GetObjectItem(obj, "balance")->valuestring;
    uint64_t tmp_balance = strtoull(balance, NULL, 10);
    int offset = cJSON_GetObjectItem(obj, "offset")->valueint;


    cJSON* address_json_generated = generate_new_addresses(seed, offset, 1+offset);
    char* generated_address = cJSON_GetObjectItem(
                                           cJSON_GetArrayItem(address_json_generated, 0),
                                           "address"
                              )->valuestring;

    if(strcmp(address, generated_address) != 0) { //Not correct inputs...
      log_wallet_error("Input Address Mismatch. Offset: %d (found %s vs supplied %s) ", offset, generated_address, address );

    }


    cJSON_AddItemToArray(inputs_to_use,
        cJSON_Duplicate(obj, 1)
      );
    running_balance += tmp_balance;
    if(running_balance >= value) {
      break;
    }
  }
  cJSON_Delete(inputs);



  //Sign the tx

  char* trytes = send_transaction(
                                    seed,
                                    dest_address,
                                    change_address,
                                    value,
                                    inputs_to_use
                  );


  if(!trytes) {
    log_wallet_error("Error creating transaction, could not sign", "");
    cJSON_Delete(change_address_json);
    cJSON_Delete(inputs_to_use);
    close_db_handle(db);
    if(free_user == 1) {
      free(u);
    }
    return -1;
  }


  //create an outgoing tx
  //store inputs as trytes, the thread will replace with actual tx trytes
  int create = create_outgoing_transaction(
                                            db,
                                            dest_address,
                                            change_address,
                                            value,
                                            trytes
                );

  free(trytes);

  if(create == 0) { //Success
    cJSON_ArrayForEach(obj, inputs_to_use) {
      char* address = cJSON_GetObjectItem(obj, "address")->valuestring;
      mark_address_spent_from(db, address);
    }
  }

  cJSON_Delete(inputs_to_use);

  cJSON_Delete(change_address_json);
  close_db_handle(db);
  if(free_user == 1) {
    free(u);
  }
  return create;
}
