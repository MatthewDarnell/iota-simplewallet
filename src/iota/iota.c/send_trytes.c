//
// Created by matth on 3/3/2020.
//

#include <cclient/api/core/core_api.h>
#include <cclient/api/extended/extended_api.h>
#include "../../iota-simplewallet.h"
#include "../api.h"

static retcode_t json_array_to_hash8019_array(cJSON const* const obj, char const* const obj_name, hash8019_array_p array) {
  flex_trit_t hash[FLEX_TRIT_SIZE_8019] = { 0 };
  cJSON* json_item = cJSON_GetObjectItemCaseSensitive(obj, obj_name);
  if (cJSON_IsArray(json_item)) {
    cJSON* current_obj = NULL;
    cJSON_ArrayForEach(current_obj, json_item) {
      if (current_obj->valuestring != NULL) {
        flex_trits_from_trytes(hash, NUM_TRITS_SERIALIZED_TRANSACTION, (tryte_t const*)current_obj->valuestring,
                               NUM_TRYTES_SERIALIZED_TRANSACTION, NUM_TRYTES_SERIALIZED_TRANSACTION);
        hash_array_push(array, hash);
      }
    }
  } else {
    log_wallet_error("[%s:%d] %s not array\n", __func__, __LINE__, STR_CCLIENT_JSON_PARSE);
    return RC_CCLIENT_JSON_PARSE;
  }
  return RC_OK;
}

int send_trytes(char* out_bundle, int out_bundle_max_len, char* out_hash, int out_hash_max_len, uint64_t serial, const char* trytes) {
  retcode_t ret_code = RC_OK;

  uint32_t depth = 3;
  uint8_t mwm = 14;

  iota_client_service_t* serv = get_iota_client();
  transaction_array_t *out_tx_objs = transaction_array_new();
  hash8019_array_p raw_trytes = hash8019_array_new();

  if (!serv || !out_tx_objs || !raw_trytes) {
    log_wallet_error("%s OOM\n", __func__);
    return -1;
  }

  cJSON* json_trytes = cJSON_Parse(trytes);
  if(!json_trytes) {
    log_wallet_error("Invalid Trytes %s\n", trytes);
    transaction_array_free(out_tx_objs);
    iota_client_core_destroy(&serv);
    hash_array_free(raw_trytes);
    return -1;
  }

  if(!cJSON_HasObjectItem(json_trytes, "trytes")) {
    log_wallet_error("Invalid Trytes %s\n", trytes);
    cJSON_Delete(json_trytes);
    transaction_array_free(out_tx_objs);
    iota_client_core_destroy(&serv);
    hash_array_free(raw_trytes);
    return -1;
  }

  hash8019_array_p array = hash8019_array_new();

   if(RC_OK != json_array_to_hash8019_array(json_trytes, "trytes", array)) {
     log_wallet_error("error converting trytes\n", "")
   }

  cJSON_Delete(json_trytes);

  int ret_val = 0;
  if ((ret_code = iota_client_send_trytes(serv, array, depth, mwm, NULL, false,
                                          out_tx_objs)) == RC_OK) {
    log_wallet_info("Transaction has been sent\n", "");

    tryte_t hash[NUM_TRYTES_HASH + 1] = { 0 };
    tryte_t bundle[NUM_TRYTES_BUNDLE + 1] = { 0 };

    iota_transaction_t* tx = transaction_array_at(out_tx_objs, 0);
    memset(hash, 0, NUM_TRYTES_HASH + 1);
    memset(bundle, 0, NUM_TRYTES_BUNDLE + 1);

    flex_trit_t* trit_hash = transaction_hash(tx);
    flex_trit_t* trit_bundle = transaction_bundle(tx);

    flex_trits_to_trytes(
      hash, NUM_TRYTES_HASH,
      trit_hash,
      NUM_TRITS_HASH, NUM_TRITS_HASH);

    flex_trits_to_trytes(
      bundle, NUM_TRYTES_BUNDLE,
      trit_bundle,
      NUM_TRITS_BUNDLE, NUM_TRITS_BUNDLE);

    size_t bundle_len_to_copy = strlen((char*)bundle);
    size_t hash_len_to_copy = strlen((char*)hash);

    memset(out_bundle, 0, out_bundle_max_len);
    memset(out_hash, 0, out_hash_max_len);

    memcpy(out_bundle, (char*)bundle, bundle_len_to_copy);
    memcpy(out_hash, (char*)hash, hash_len_to_copy);

  } else {
    log_wallet_error("Error Sending Trytes: %s\n", error_2_string(ret_code));
    ret_val = -1;
  }

  transaction_array_free(out_tx_objs);
  hash_array_free(raw_trytes);
  iota_client_core_destroy(&serv);
  return ret_val;
}