//
// Created by matth on 3/3/2020.
//

#include <cclient/api/core/core_api.h>
#include <cclient/api/extended/extended_api.h>
#include "../../config/logger.h"
#include "../api.h"

int send_trytes(char* out_bundle, int out_bundle_max_len, char* out_hash, int out_hash_max_len, uint64_t serial, const char* trytes) {
  retcode_t ret_code = RC_OK;
  flex_trit_t trits_8019[FLEX_TRIT_SIZE_8019 + 1];

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
  cJSON* trytes_array = cJSON_GetObjectItem(json_trytes, "trytes");
  cJSON* tryte_obj = NULL;

  cJSON_ArrayForEach(tryte_obj, trytes_array) {
    memset(trits_8019, 0, FLEX_TRIT_SIZE_8019 + 1);
    if (flex_trits_from_trytes(trits_8019, NUM_TRITS_SERIALIZED_TRANSACTION, (tryte_t*)cJSON_GetStringValue(tryte_obj),
                               NUM_TRYTES_SERIALIZED_TRANSACTION, NUM_TRYTES_SERIALIZED_TRANSACTION) == 0) {
      log_wallet_error("Converting flex_trit failed. %s\n", cJSON_Print(tryte_obj));
      continue;
    }
    hash_array_push(raw_trytes, trits_8019);
  }

  int ret_val = 0;
  if ((ret_code = iota_client_send_trytes(serv, raw_trytes, depth, mwm, NULL, false,
                                          out_tx_objs)) == RC_OK) {
    log_wallet_info("Transaction has been sent\n", "");
    iota_transaction_t *tx_obj = NULL;

    tryte_t hash[NUM_TRYTES_HASH + 1] = { 0 };
    tryte_t bundle[NUM_TRYTES_BUNDLE + 1] = { 0 };

    TX_OBJS_FOREACH(out_tx_objs, tx_obj) {
      memset(hash, 0, NUM_TRYTES_HASH + 1);
      memset(bundle, 0, NUM_TRYTES_BUNDLE + 1);

      flex_trit_t* trit_hash = transaction_hash(tx_obj);
      flex_trit_t* trit_bundle = transaction_hash(tx_obj);

      flex_trits_to_trytes(
        hash, NUM_TRYTES_HASH,
        trit_hash,
        NUM_TRITS_HASH, NUM_TRITS_HASH);

      flex_trits_to_trytes(
        bundle, NUM_TRYTES_BUNDLE,
        trit_bundle,
        NUM_TRITS_BUNDLE, NUM_TRITS_BUNDLE);

      int bundle_len_to_copy = strlen((char*)bundle) > out_bundle_max_len ? out_bundle_max_len : strlen((char*)bundle);
      int hash_len_to_copy = strlen((char*)hash) > out_hash_max_len ? out_hash_max_len : strlen((char*)hash);

      memset(out_bundle, 0, (size_t)out_bundle_max_len);
      memset(out_hash, 0, (size_t)out_hash_max_len);

      memcpy(out_bundle, (char*)bundle, bundle_len_to_copy);
      memcpy(out_hash, (char*)hash, hash_len_to_copy);

    }
  } else {
    log_wallet_error("Error Sending Trytes: %s\n", error_2_string(ret_code));
    ret_val = -1;
  }



  transaction_array_free(out_tx_objs);
  hash_array_free(raw_trytes);
  iota_client_core_destroy(&serv);
  return ret_val;
}