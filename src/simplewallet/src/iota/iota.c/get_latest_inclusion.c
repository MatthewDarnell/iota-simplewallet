//
// Created by matth on 2/25/2020.
//

#include <cclient/service.h>
#include <cclient/api/core/core_api.h>
#include <cclient/api/extended/extended_api.h>
#include "../../iota-simplewallet.h"
#include "../api.h"

void get_latest_inclusion(cJSON** addresses_with_transactions, int include_unconfirmed) {
  iota_client_service_t* serv = get_iota_client();
  if(!serv) {
    log_wallet_error( "%s oom\n", __func__);
    return;
  }
  retcode_t ret = RC_ERROR;
  flex_trit_t tmp_hash[FLEX_TRIT_SIZE_243];
  hash243_queue_t txs = NULL;

  int i,j;
  for(i=0; i < cJSON_GetArraySize(*addresses_with_transactions); i++) {

    cJSON* address_object = cJSON_GetArrayItem(*addresses_with_transactions, i);
    if(!cJSON_HasObjectItem(address_object, "transactions")) {
      continue;
    }

    cJSON* transaction_array = cJSON_GetObjectItem(address_object, "transactions");
    size_t num_txs = cJSON_GetArraySize(transaction_array);

    if(num_txs < 1) {
      continue;
    }

    get_inclusion_states_res_t *inclusion_res = get_inclusion_states_res_new();
    if (!inclusion_res) {
      log_wallet_error( "%s Error: OOM\n", __func__);
      continue;
    }


    for(j = 0; j < num_txs; j++) {
      cJSON* obj = cJSON_GetArrayItem(transaction_array, j);
      const char* hash = cJSON_GetObjectItem(obj, "hash")->valuestring;
      flex_trits_from_trytes(tmp_hash, NUM_TRITS_HASH, (tryte_t*)hash, NUM_TRYTES_HASH, NUM_TRYTES_HASH);
      hash243_queue_push(&txs, tmp_hash);
    }


    if ((ret = iota_client_get_latest_inclusion(serv, txs, inclusion_res)) != RC_OK) {
      log_wallet_error("%s, %s\n", __func__, error_2_string(ret));
      get_inclusion_states_res_free(&inclusion_res);
      hash243_queue_free(&txs);
      continue;
    }

    for (j = 0; j < (int)get_inclusion_states_res_states_count(inclusion_res); j++) {
      cJSON* obj = cJSON_GetArrayItem(transaction_array, j);
     // int confirmed = (int)get_inclusion_states_res_states_at(inclusion_res, j);
      cJSON_AddStringToObject(obj, "confirmed", get_inclusion_states_res_states_at(inclusion_res, j) ? "true" : "false");
    }

    if(!include_unconfirmed) {
      j = 0;
      while(1) {
        if(j >=  cJSON_GetArraySize(transaction_array)) {
          break;
        }
        cJSON* obj = cJSON_GetArrayItem(transaction_array, j);
        char* c = cJSON_GetObjectItem(obj, "confirmed")->valuestring;
        if(strcasecmp(c, "false") == 0) {
          cJSON_DeleteItemFromArray(transaction_array, j);
          j--;
        }
        j++;
      }
    }


    get_inclusion_states_res_free(&inclusion_res);
    hash243_queue_free(&txs);
  }

  free_iota_client(&serv);
}


void get_latest_inclusion_by_tx_hash(cJSON** tx_array) {
  iota_client_service_t* serv = NULL;
  serv = get_iota_client();
  if(!serv) {
    log_wallet_error( "%s unable  to get iota node\n", __func__);
    return;
  }

  retcode_t ret = RC_ERROR;
  flex_trit_t trits_243[FLEX_TRIT_SIZE_243];
  hash243_queue_t txs = NULL;

  get_inclusion_states_res_t *inclusion_res = get_inclusion_states_res_new();
  if (!inclusion_res) {
    log_wallet_error("%s OOM", __func__);
    iota_client_core_destroy(&serv);
    return;
  }


  cJSON* tx = NULL;

  cJSON_ArrayForEach(tx, *tx_array) {
    memset(trits_243, 0, FLEX_TRIT_SIZE_243);
    char* hash = cJSON_GetObjectItem(tx, "hash")->valuestring;
    if (flex_trits_from_trytes(trits_243, NUM_TRITS_HASH, (tryte_t*)hash, NUM_TRYTES_HASH, NUM_TRYTES_HASH) == 0) {
      log_wallet_error("%s could not convert hash to flex trit", __func__);
      get_inclusion_states_res_free(&inclusion_res);
      iota_client_core_destroy(&serv);
      return;
    }
    hash243_queue_push(&txs, trits_243);
  }



  if ((ret = iota_client_get_latest_inclusion(serv, txs, inclusion_res)) == RC_OK) {
    for (size_t i = 0; i < get_inclusion_states_res_states_count(inclusion_res); i++) {
      int confirmed = get_inclusion_states_res_states_at(inclusion_res, i);
      tx = cJSON_GetArrayItem(*tx_array, i);
      if(cJSON_HasObjectItem(tx, "confirmed")) {
        cJSON_DeleteItemFromObject(tx, "confirmed");
      }
      cJSON_AddNumberToObject(tx, "confirmed", confirmed);
    }
  } else {
    log_wallet_error("%s: %s\n", __func__, error_2_string(ret));
  }

  get_inclusion_states_res_free(&inclusion_res);
  hash243_queue_free(&txs);
  iota_client_core_destroy(&serv);
}

