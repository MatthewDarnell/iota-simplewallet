//
// Created by matth on 2/19/2020.
//
#include <cclient/api/core/core_api.h>
#include <cclient/api/extended/extended_api.h>
#include "../api.h"
#include "../../iota-simplewallet.h"

void get_transaction_inputs_to_address(cJSON** addresses) {
  iota_client_service_t* serv = NULL;
  serv = get_iota_client();
  if(!serv) {
    log_wallet_error( "%s unable  to get iota node\n", __func__);
    return;
  }
  find_transactions_req_t *find_tran = find_transactions_req_new();
  transaction_array_t *out_tx_objs = transaction_array_new();

  if (!find_tran || !out_tx_objs) {
    log_wallet_error( "Error getting tx inputs\n", "");
    iota_client_core_destroy(&serv);
    return;
  }

  flex_trit_t tmp_hash[FLEX_TRIT_SIZE_243] = { 0 };

  int i, j;
  for(i = 0; i < cJSON_GetArraySize(*addresses); i++) {
    memset(tmp_hash, 0, FLEX_TRIT_SIZE_243);
    char* address = cJSON_GetObjectItem(
                            cJSON_GetArrayItem(*addresses, i),
                            "address"
                          )->valuestring;
    if(
      0 == flex_trits_from_trytes(
                                    tmp_hash, NUM_TRITS_HASH,
                                    (const tryte_t *)address,
                                    NUM_TRYTES_HASH, NUM_TRYTES_HASH
                                  )
      )
    {
        log_wallet_error( "Unable to convert address %s to trytes\n", address);
        find_transactions_req_free(&find_tran);
        transaction_array_free(out_tx_objs);
        iota_client_core_destroy(&serv);
        return;
    }
    retcode_t push;
    if ((push = hash243_queue_push(&find_tran->addresses, tmp_hash)) != RC_OK) {
      log_wallet_error( "Error: push queue %s\n", error_2_string(push));
      find_transactions_req_free(&find_tran);
      transaction_array_free(out_tx_objs);
      iota_client_core_destroy(&serv);
      return;
    }
  }

  retcode_t ret_code;

  if ((ret_code = iota_client_find_transaction_objects(serv, find_tran, out_tx_objs)) != RC_OK) {
    log_wallet_error( "Error finding Transactions:  %s\n", error_2_string(ret_code));
    find_transactions_req_free(&find_tran);
    iota_client_core_destroy(&serv);
    transaction_array_free(out_tx_objs);
    return;
  }



  size_t count = transaction_array_len(out_tx_objs);
  if(count <= 0) {
    log_wallet_error( "Found %d inputs\n", count);
    find_transactions_req_free(&find_tran);
    iota_client_core_destroy(&serv);
    transaction_array_free(out_tx_objs);
    return;
  }
  tryte_t hash[NUM_TRYTES_HASH + 1] = { 0 };
  tryte_t address[NUM_TRYTES_ADDRESS + 1 ] = { 0 };
  tryte_t bundle[NUM_TRYTES_BUNDLE + 1 ] = { 0 };

  char time[128] = { 0 };
  char  value[64] = { 0 };

  for (i = 0; i < count; i++) {
    memset(hash, 0, NUM_TRYTES_HASH + 1);
    memset(address, 0, NUM_TRYTES_ADDRESS + 1);
    memset(bundle, 0, NUM_TRYTES_BUNDLE + 1);

    iota_transaction_t* tx = transaction_array_at(out_tx_objs, i);

    flex_trit_t* trit_address = transaction_address(tx);
    flex_trit_t* trit_hash = transaction_hash(tx);
    flex_trit_t* trit_bundle = transaction_bundle(tx);
    uint64_t t_time  = transaction_timestamp(tx);
    int64_t d_value = transaction_value(tx);




    if(d_value <= 0) {
      continue;
    }

    memset(time, 0, 128);
    memset(value, 0, 64);

#ifdef WIN32
    snprintf(time, 128, "%I64u", t_time);
    snprintf(value, 64, "%I64d", d_value);
#else
    snprintf(time, 128, "%llu", t_time);
    snprintf(value, 64, "%lld", d_value);
#endif


    flex_trits_to_trytes(
      hash, NUM_TRYTES_HASH,
      trit_hash,
      NUM_TRITS_HASH, NUM_TRITS_HASH);

    flex_trits_to_trytes(
      address, NUM_TRYTES_ADDRESS,
      trit_address,
      NUM_TRITS_ADDRESS, NUM_TRITS_ADDRESS);

    flex_trits_to_trytes(
      bundle, NUM_TRYTES_BUNDLE,
      trit_bundle,
      NUM_TRITS_BUNDLE, NUM_TRITS_BUNDLE);

    for(j = 0; j < cJSON_GetArraySize(*addresses); j++) {
      cJSON* addr_obj = cJSON_GetArrayItem(*addresses, j);
      const char* addr_address = cJSON_GetObjectItem(addr_obj, "address")->valuestring;
      if(strcasecmp(addr_address, (const char*)address) == 0) {
        if(!cJSON_HasObjectItem(addr_obj, "transactions")) {
          cJSON_AddItemToObject(addr_obj, "transactions", cJSON_CreateArray());
        }
        cJSON* addr_obj_txs = cJSON_GetObjectItem(addr_obj, "transactions");

        cJSON* obj = cJSON_CreateObject();
        cJSON_AddStringToObject(obj, "hash", (const char*)hash);
        cJSON_AddStringToObject(obj, "bundle", (const char*)bundle);
        cJSON_AddStringToObject(obj, "amount", (const char*)value);
        cJSON_AddStringToObject(obj, "timestamp", (const char*)time);

        cJSON_AddItemToArray(addr_obj_txs, obj);
        break;
      }
    }
  }

  j = 0;
  while(1) {
    if(j >=  cJSON_GetArraySize(*addresses)) {
      break;
    }
    cJSON* obj = cJSON_GetArrayItem(*addresses, j);
    if(!cJSON_HasObjectItem(obj, "transactions")) {
      cJSON_DeleteItemFromArray(*addresses, j);
      j--;
    }
    j++;
  }

  find_transactions_req_free(&find_tran);
  transaction_array_free(out_tx_objs);
  iota_client_core_destroy(&serv);
}