//
// Created by Matthew Darnell on 3/30/20.
//

#include <cclient/service.h>
#include <cclient/api/core/core_api.h>
#include <cclient/api/extended/extended_api.h>
#include "../../iota-simplewallet.h"
#include "../api.h"

cJSON* find_transactions_by_bundle(const char* bundle) {
  iota_client_service_t* serv = get_iota_client();
  if(!serv) {
    log_wallet_error( "%s oom\n", __func__);
    return NULL;
  }

  retcode_t ret_code = RC_ERROR;
  find_transactions_req_t *find_tran = find_transactions_req_new();
  transaction_array_t *out_tx_objs = transaction_array_new();
  if (!find_tran || !out_tx_objs) {
    printf("Error: OOM\n");
    free_iota_client(&serv);
    return NULL;
  }

  // find transaction by bundle
  flex_trit_t tmp_bundle[FLEX_TRIT_SIZE_243];
  if (flex_trits_from_trytes(tmp_bundle, NUM_TRITS_HASH, (const tryte_t*)bundle, NUM_TRYTES_HASH, NUM_TRYTES_HASH) == 0) {
    printf("Error: converting flex_trit failed.\n");
    free_iota_client(&serv);
    find_transactions_req_free(&find_tran);
    transaction_array_free(out_tx_objs);
    return NULL;
  }

  if ((ret_code = hash243_queue_push(&find_tran->bundles, tmp_bundle)) != RC_OK) {
    printf("Error: push queue %s\n", error_2_string(ret_code));
    find_transactions_req_free(&find_tran);
    transaction_array_free(out_tx_objs);
    free_iota_client(&serv);
    return NULL;
  }


  cJSON* ret_val = NULL;
  if ((ret_code = iota_client_find_transaction_objects(serv, find_tran, out_tx_objs)) == RC_OK) {
    int num_txs = transaction_array_len(out_tx_objs);
    if(num_txs > 0) {
      ret_val = cJSON_CreateArray();
      int i;
      for(i = 0; i < num_txs; i++) {
        iota_transaction_t* tx = transaction_array_at(out_tx_objs, i);
        if(transaction_current_index(tx) == 0) {
          flex_trit_t* hash = transaction_hash(tx);
          char h[256] = { 0 };
          flex_trits_to_trytes(
            (tryte_t*)h, NUM_TRYTES_HASH,
            hash,
            NUM_TRITS_HASH, NUM_TRITS_HASH);
          cJSON_AddItemToArray(ret_val, cJSON_CreateString(h));
        }
      }
    }
  } else {
    fprintf(stderr, "Error: %s \n", error_2_string(ret_code));
  }

  find_transactions_req_free(&find_tran);
  transaction_array_free(out_tx_objs);
  free_iota_client(&serv);

  return ret_val;
}