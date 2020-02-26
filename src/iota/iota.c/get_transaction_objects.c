//
// Created by matth on 2/22/2020.
//

#include <unity/unity.h>
#include <common/model/transaction.h>
#include "common/trinary/tryte.h"
#include "../api.h"
#include "../config/http.h"
#include "core_client.h"

cJSON* get_transaction_objects(cJSON* trytes) {
  cJSON* inputs = NULL;
  cJSON* t_array = cJSON_GetObjectItem(trytes, "trytes");
  flex_trit_t tmp_tx[FLEX_TRIT_SIZE_8019];
  int i;
  for(i = 0; i < cJSON_GetArraySize(t_array); i++) {
    const char* tryte = cJSON_GetArrayItem(t_array, i)->valuestring;
    flex_trits_from_trytes(tmp_tx, NUM_TRITS_SERIALIZED_TRANSACTION, (tryte_t *)tryte, NUM_TRITS_SERIALIZED_TRANSACTION,
                           NUM_TRYTES_SERIALIZED_TRANSACTION);


    iota_transaction_t *tx = transaction_deserialize(tmp_tx, true);


    printf("\nreturned tx:\n");
    printf("size of transaction object actually returned: %I64d\n", sizeof(iota_transaction_t));


    transaction_obj_dump(tx);
    printf("\nGetting value: ");
    int64_t value = transaction_value(tx);
    flex_trit_t* hash = transaction_hash(tx);

    char hash_trytes[243] = { 0 };
    flex_trits_to_trytes((tryte_t*)hash_trytes, NUM_TRYTES_HASH, hash, NUM_TRITS_HASH, NUM_TRITS_HASH);

    printf("\n hash: %s\nvalue: %I64d\n", hash_trytes, value);


  }


  return inputs;
}