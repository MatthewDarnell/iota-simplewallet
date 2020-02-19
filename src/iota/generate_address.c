//
// Created by matth on 2/19/2020.
//

#include <common/trinary/tryte.h>
#include  <cclient/api/extended/get_new_address.h>
#include <cclient/api/core/core_api.h>
#include "../config/config.h"
#include "generate_address.h"

cJSON* get_new_address(const char* seed, int index, int num_addresses) {
  iota_client_service_t *serv = NULL;

  char* str_nodes = get_config("nodes");
  cJSON* nodes = cJSON_Parse(str_nodes);
  free(str_nodes);

  if(cJSON_GetArraySize(nodes) < 1) {
    fprintf(stderr, "Unable to get nodes\n");
    cJSON_Delete(nodes);
    return NULL;
  }
  cJSON* node = cJSON_GetArrayItem(nodes, 0);

  const char* host = cJSON_GetObjectItem(node, "host")->valuestring;
  const int port = atoi(cJSON_GetObjectItem(node, "port")->valuestring);
  const char* pem = cJSON_GetObjectItem(node, "pem")->valuestring;

  serv = iota_client_core_init(host, port, pem);

  cJSON_Delete(nodes);

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
  return json_addresses;
}