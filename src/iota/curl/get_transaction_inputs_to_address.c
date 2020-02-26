//
// Created by matth on 2/25/2020.
//

#include <sodium.h>
#include <common/model/transaction.h>
#include "../../config/logger.h"
#include "../../config/http.h"
#include "../api.h"

static cJSON* get_trytes(cJSON* object) {
  printf("%s\n", __func__);
  cJSON* trytes = NULL;
  cJSON* request = cJSON_CreateObject();
  cJSON_AddStringToObject(request, "command", "getTrytes");
  cJSON_AddItemToObject(request, "hashes", cJSON_GetObjectItem(object, "hashes"));
  char* query_string = cJSON_PrintUnformatted(request);
  cJSON_Delete(request);
  int p_ret = post(query_string, &trytes);
  free(query_string);
  if(p_ret < 0) {
    log_wallet_error("Get Trytes failed with %d", p_ret);
    return NULL;
  }
  if(!trytes) {
    log_wallet_error("Get Trytes failed, return value NULL", "")
    return NULL;
  }
  if(!cJSON_HasObjectItem(trytes, "trytes")) {
    log_wallet_error("Get Trytes failed, returned %s", cJSON_Print(trytes));
    cJSON_Delete(trytes);
    return NULL;
  }
  return trytes;
}


void get_transaction_inputs_to_address(cJSON** addresses) {
  printf("%s\n", __func__);
  cJSON *request = cJSON_CreateObject();
  cJSON *response = NULL;

  cJSON_AddStringToObject(request, "command", "findTransactions");
  cJSON_AddItemToObject(request, "addresses", cJSON_CreateArray());
  cJSON *address_array = cJSON_GetObjectItem(request, "addresses");


  int i, j;
  for (i = 0; i < cJSON_GetArraySize(*addresses); i++) {
    const char *address = cJSON_GetObjectItem(cJSON_GetArrayItem(*addresses, i), "address")->valuestring;
    cJSON_AddItemToArray(address_array, cJSON_CreateString(address));
  }

  char *request_string = cJSON_PrintUnformatted(request);
  cJSON_Delete(request);
  int p_ret = post(request_string, &response);
  free(request_string);

  if (!response) {
    log_wallet_error("Find Transactions http request returned %d", p_ret);
    return;
  }

  if(!cJSON_HasObjectItem(response, "hashes")) {
    cJSON_Delete(response);
    return;
  }
 // const char* r = cJSON_Print(response);
  //printf("\nTx Inputs:\n%s\n",r);

  cJSON* trytes_response = NULL;
  if((trytes_response = get_trytes(response)) == NULL) {
    cJSON_Delete(response);
    return;
  }
  if(!cJSON_HasObjectItem(trytes_response, "trytes")) {
    if(response) {
      cJSON_Delete(response);
    }
    return;
  }

  cJSON* trytes = cJSON_GetObjectItem(trytes_response, "trytes");


  tryte_t hash[NUM_TRYTES_HASH + 1] = { 0 };
  tryte_t address[NUM_TRYTES_ADDRESS + 1 ] = { 0 };
  tryte_t bundle[NUM_TRYTES_BUNDLE + 1 ] = { 0 };

  char time[128] = { 0 };
  char  value[64] = { 0 };

  flex_trit_t tmp_tx[FLEX_TRIT_SIZE_8019] = { 0 };

  for(i = 0; i < cJSON_GetArraySize(trytes); i++) {

    memset(hash, 0, NUM_TRYTES_HASH + 1);
    memset(address, 0, NUM_TRYTES_ADDRESS + 1);
    memset(bundle, 0, NUM_TRYTES_BUNDLE + 1);
    memset(time, 0, 128);
    memset(value, 0, 64);


    const char* tryte = cJSON_GetArrayItem(trytes, i)->valuestring;
    flex_trits_from_trytes(tmp_tx, NUM_TRITS_SERIALIZED_TRANSACTION, (tryte_t *)tryte, NUM_TRITS_SERIALIZED_TRANSACTION,
                           NUM_TRYTES_SERIALIZED_TRANSACTION);

    iota_transaction_t *tx = transaction_deserialize(tmp_tx, true);

    flex_trit_t* trit_address = transaction_address(tx);
    flex_trit_t* trit_hash = transaction_hash(tx);
    flex_trit_t* trit_bundle = transaction_bundle(tx);
    uint64_t t_time  = transaction_timestamp(tx);
    int64_t d_value = transaction_value(tx);

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

    cJSON* obj = cJSON_CreateObject();
    cJSON_AddStringToObject(obj, "hash", (const char*)hash);
    cJSON_AddStringToObject(obj, "bundle", (const char*)bundle);
    cJSON_AddStringToObject(obj, "amount", (const char*)value);
    cJSON_AddStringToObject(obj, "timestamp", (const char*)time);

    for(j = 0; j < cJSON_GetArraySize(*addresses); j++) {
      cJSON* addr_obj = cJSON_GetArrayItem(*addresses, j);
      if(!cJSON_HasObjectItem(addr_obj, "transactions")) {
        cJSON_AddItemToObject(addr_obj, "transactions", cJSON_CreateArray());
      }

      const char* addr_address = cJSON_GetObjectItem(addr_obj, "address")->valuestring;
      if(strcasecmp(addr_address, (const char*)address) == 0) {

        cJSON* addr_obj_txs = cJSON_GetObjectItem(addr_obj, "transactions");
        cJSON_AddItemToArray(addr_obj_txs, obj);
        break;
      }
    }
  }
  cJSON_Delete(trytes_response);
}
