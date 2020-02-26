//
// Created by matth on 2/25/2020.
//

#include <cclient/service.h>
#include <cclient/api/extended/extended_api.h>
#include "../api.h"

void get_latest_inclusion(cJSON** addresses_with_transactions, int include_unconfirmed) {
  iota_client_service_t* serv = get_iota_client();
  if(!serv) {
    fprintf(stderr, "%s oom\n", __func__);
    return;
  }
  retcode_t ret = RC_ERROR;
  flex_trit_t tmp_hash[FLEX_TRIT_SIZE_243];
  hash243_queue_t txs = NULL;

  int i,j;
  for(i=0; i < cJSON_GetArraySize(*addresses_with_transactions); i++) {

    get_inclusion_states_res_t *inclusion_res = get_inclusion_states_res_new();
    if (!inclusion_res) {
      fprintf(stderr, "%s Error: OOM\n", __func__);
      continue;
    }


    cJSON* address_object = cJSON_GetArrayItem(*addresses_with_transactions, i);
    if(!cJSON_HasObjectItem(address_object, "transactions")) {
      continue;
    }

    cJSON* transaction_array = cJSON_GetObjectItem(address_object, "transactions");
    for(j = 0; j < cJSON_GetArraySize(transaction_array); j++) {
      cJSON* obj = cJSON_GetArrayItem(transaction_array, j);
      const char* hash = cJSON_GetObjectItem(obj, "hash")->valuestring;
      flex_trits_from_trytes(tmp_hash, NUM_TRITS_HASH, (tryte_t*)hash, NUM_TRYTES_HASH, NUM_TRYTES_HASH);
      hash243_queue_push(&txs, tmp_hash);
    }


    if ((ret = iota_client_get_latest_inclusion(serv, txs, inclusion_res)) != RC_OK) {
      fprintf(stderr,"%s in get_latest_inclusion: Error: %s\n", __func__, error_2_string(ret));
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