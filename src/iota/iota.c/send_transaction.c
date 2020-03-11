//
// Created by matth on 2/26/2020.
//

#include <stdio.h>
#include <sodium.h>
#include <cjson/cJSON.h>
#include <cclient/api/core/core_api.h>
#include <common/trinary/flex_trit.h>
#include <common/model/transfer.h>
#include <common/model/bundle.h>
#include <cclient/api/extended/extended_api.h>
#include "../../config/config.h"
#include "../../config/logger.h"
#include "../api.h"

//was including serialization/json/helpers.h for this function, but it includes local "cJSON.h" and that was failing
static retcode_t hash8019_array_to_json_array(hash8019_array_p array, cJSON* const json_root, char const* const obj_name) {
  size_t array_count = 0;
  cJSON* array_obj = NULL;
  tryte_t trytes_out[NUM_TRYTES_SERIALIZED_TRANSACTION] = { 0 };
  size_t trits_count = 0;
  flex_trit_t* elt = NULL;

  array_count = hash_array_len(array);
  if (array_count > 0) {
    array_obj = cJSON_CreateArray();
    if (array_obj == NULL) {
      return RC_CCLIENT_JSON_CREATE;
    }
    cJSON_AddItemToObject(json_root, obj_name, array_obj);

    HASH_ARRAY_FOREACH(array, elt) {
      trits_count = flex_trits_to_trytes(trytes_out, NUM_TRYTES_SERIALIZED_TRANSACTION, elt,
                                         NUM_TRITS_SERIALIZED_TRANSACTION, NUM_TRITS_SERIALIZED_TRANSACTION);
      //trytes_out[NUM_TRYTES_SERIALIZED_TRANSACTION] = '\0';
      if (trits_count != 0) {
        cJSON_AddItemToArray(array_obj, cJSON_CreateString((char const*)trytes_out));
      } else {
        return RC_CCLIENT_FLEX_TRITS;
      }
    }
  }
  return RC_OK;
}

//inputs:
/*
 [
  {
    address,
    balance,
    offset
  }
 ]
*/
char* send_transaction(char* seed, const char* dest_address, const char* change_address, uint64_t value, cJSON* inputs) {
  //validate inputs

  if(!cJSON_IsArray(inputs)) {
    log_wallet_error("Invalid input (not array type!) %s", cJSON_Print(inputs));
    return NULL;
  }

  int num_inputs = cJSON_GetArraySize(inputs);
  if(num_inputs < 1) {
    log_wallet_error("Empty input array %s", cJSON_Print(inputs));
    return NULL;
  }
  cJSON* input_obj = NULL;

  uint64_t sum_inputs = 0;  //Don't want to include more than we are sending
  cJSON_ArrayForEach(input_obj, inputs) {
    if(
      !cJSON_HasObjectItem(input_obj, "address") ||
      !cJSON_HasObjectItem(input_obj, "balance") ||
      !cJSON_HasObjectItem(input_obj, "offset")
    ) {
      log_wallet_error("Invalid input array %s", cJSON_Print(inputs));
      return NULL;
    } else {
      const char* balance = cJSON_GetObjectItem(input_obj, "balance")->valuestring;
      uint64_t d_balance = strtoull(balance, NULL, 10);
      sum_inputs += d_balance;
      if(sum_inputs >= value) {
        break;
      }
    }
  }
  if(sum_inputs < value) {
#ifdef WIN32
    log_wallet_error("Not enough funds! Sending.(%I64u i), only have (%I64u i)", value, sum_inputs);
#else
    log_wallet_error("Not enough funds! Sending.(%llu i), only have (%llu i)", value, sum_inputs);
#endif
    return NULL;
  }

  sum_inputs = 0;



  iota_client_service_t* serv = get_iota_client();

  retcode_t ret_code = RC_OK;

  uint8_t security = 2;

  bundle_transactions_t *bundle = NULL;
  bundle_transactions_new(&bundle);
  transfer_array_t *transfers = transfer_array_new();

  /* transfer setup */
  transfer_t tf = {};

  // seed
  flex_trit_t trit_seed[NUM_FLEX_TRITS_ADDRESS];
  flex_trits_from_trytes(trit_seed, NUM_TRITS_ADDRESS, (tryte_t*)seed, NUM_TRYTES_ADDRESS, NUM_TRYTES_ADDRESS);
  sodium_memzero(seed, strlen(seed));

  // receiver
  flex_trits_from_trytes(tf.address, NUM_TRITS_ADDRESS, (tryte_t*)dest_address, NUM_TRYTES_ADDRESS, NUM_TRYTES_ADDRESS);

  // tag
  flex_trits_from_trytes(tf.tag, NUM_TRITS_TAG, (const tryte_t *)"IOTACWALLET9999999999999999", NUM_TRYTES_TAG,
                         NUM_TRYTES_TAG);

  // value
  tf.value = value; // send i to receiver
  transfer_array_add(transfers, &tf);


  /* input setup */
  inputs_t input_list = {};  // input list

  //for loop over inputs:
  cJSON_ArrayForEach(input_obj, inputs) {
    const char* address = cJSON_GetObjectItem(input_obj, "address")->valuestring;
    const char* balance = cJSON_GetObjectItem(input_obj, "balance")->valuestring;
    int offset = cJSON_GetObjectItem(input_obj, "offset")->valueint;

    uint64_t d_balance = strtoull(balance, NULL, 10);
    sum_inputs += d_balance;

    input_t next_input;

    next_input.balance = d_balance;
    next_input.key_index = offset;
    next_input.security = 2;

    flex_trits_from_trytes(
      next_input.address, NUM_TRITS_ADDRESS,
      (const tryte_t *)address,
      NUM_TRYTES_ADDRESS, NUM_TRYTES_ADDRESS);

    inputs_append(&input_list, &next_input);

    if(sum_inputs >= value) {
      break;
    }

  }


  /* change address */
  flex_trit_t trit_change_addr[NUM_FLEX_TRITS_ADDRESS];
  flex_trits_from_trytes(
    trit_change_addr, NUM_TRITS_ADDRESS,
    (const tryte_t *)change_address,
    NUM_TRYTES_ADDRESS, NUM_TRYTES_ADDRESS);

  if(sum_inputs == value) {
    ret_code = iota_client_prepare_transfers(serv, trit_seed, security, transfers, NULL, &input_list, true, 0, bundle);
  } else {
    ret_code = iota_client_prepare_transfers(serv, trit_seed, security, transfers, trit_change_addr, &input_list, true, 0, bundle);
  }
  sodium_memzero(trit_seed, NUM_FLEX_TRITS_ADDRESS);

  if (ret_code != RC_OK) {
    log_wallet_error("Unable to prepare transfers %s", error_2_string(ret_code));
    bundle_transactions_free(&bundle);
    transfer_message_free(&tf);
    transfer_array_free(transfers);
    iota_client_core_destroy(&serv);
    inputs_clear(&input_list);
    return NULL;
  }

  hash8019_array_p raw_tx = hash8019_array_new();
  flex_trit_t serialized_value[FLEX_TRIT_SIZE_8019];
  iota_transaction_t* tx = NULL;

  BUNDLE_FOREACH(bundle, tx) {
    // tx trytes must be in order, from last to 0.
    transaction_serialize_on_flex_trits(tx, serialized_value);
    utarray_insert(raw_tx, serialized_value, 0);
  }


  cJSON* tx_array = cJSON_CreateObject();
  hash8019_array_to_json_array(raw_tx, tx_array, "trytes");

  char* ret_val = cJSON_PrintUnformatted(tx_array);
  cJSON_Delete(tx_array);
  hash_array_free(raw_tx);

  inputs_clear(&input_list);

  ///////////////////
  bundle_transactions_free(&bundle);
  transfer_message_free(&tf);
  transfer_array_free(transfers);
  iota_client_core_destroy(&serv);
  return ret_val;
}