//
// Created by matth on 2/19/2020.
//

#include <common/trinary/tryte.h>
#include <cclient/api/core/core_api.h>
#include  <cclient/api/extended/extended_api.h>
#include "../api.h"

cJSON* get_new_address(const char* seed, int index, int num_addresses) {
  iota_client_service_t* serv = get_iota_client();
  //address options
  address_opt_t addresses_to_create;
  addresses_to_create.start = index;
  addresses_to_create.total = num_addresses;
  addresses_to_create.security = 2;

  //addresses
  hash243_queue_t addresses = NULL;

  //seed manipulation to ternary
  flex_trit_t seed_trits[FLEX_TRIT_SIZE_243] = { 0 };
  if (flex_trits_from_trytes(seed_trits, NUM_TRITS_ADDRESS, (tryte_t*)seed,  NUM_TRYTES_ADDRESS, NUM_TRYTES_ADDRESS) == 0) {
    iota_client_core_destroy(&serv);
    return NULL;
  }

  //generate address(es)
  iota_client_get_new_address(
      serv,
      seed_trits,
      addresses_to_create,
      &addresses
    );

  //trits to trytes
  hash243_queue_entry_t* out_trits = hash243_queue_pop(&addresses);

  cJSON* json_addresses = cJSON_CreateArray();

  while(out_trits != NULL) {
    tryte_t addr_trytes[128] = { 0 };
    flex_trits_to_trytes(addr_trytes, 81, out_trits->hash, 243, 243);

    cJSON* object = cJSON_CreateObject();
    cJSON_AddItemToObject(object, "address", cJSON_CreateString((const char*)addr_trytes));
    cJSON_AddItemToObject(object, "index", cJSON_CreateNumber(index));
    index++;

    cJSON_AddItemToArray(
        json_addresses,
        object
      );
    out_trits = hash243_queue_pop(&addresses);
  }
  hash243_queue_free(&addresses);
  iota_client_core_destroy(&serv);

  return json_addresses;
}