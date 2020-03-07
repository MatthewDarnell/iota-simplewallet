//
// Created by Matthew Darnell on 3/6/20.
//

#include <cjson/cJSON.h>
#include <cclient/api/extended/get_inputs.h>
#include "../api.h"
#include "../../config/logger.h"
cJSON* get_inputs(const char* seed) {
  iota_client_service_t *serv = get_iota_client();
  if (!serv) {
    log_wallet_error("Unable to get iota client", "")
    return NULL;
  }
  //address options
  address_opt_t address_opt;
  address_opt.start = 0;
  address_opt.total = 0;
  address_opt.security = 2;

  //seed manipulation to ternary
  flex_trit_t seed_trits[FLEX_TRIT_SIZE_243] = {0};
  if (flex_trits_from_trytes(seed_trits, NUM_TRITS_ADDRESS, (tryte_t *) seed, NUM_TRYTES_ADDRESS, NUM_TRYTES_ADDRESS) ==
      0) {
    free_iota_client(&serv);
    log_wallet_error("Error reading seed!", "");
    return NULL;
  }

  inputs_t inputs =  {};
  input_t* input = NULL;

  retcode_t retcode = RC_OK;
  if( (retcode = iota_client_get_inputs(serv, seed_trits, address_opt, 1, &inputs) != RC_OK)) {
    log_wallet_error("Error getting Inputs: %s\n", error_2_string(retcode));
    free_iota_client(&serv);
    return NULL;
  }

  if(inputs.total_balance <= 0) {
    free_iota_client(&serv);
    return NULL;
  }

  cJSON* json = cJSON_CreateArray();
  INPUTS_FOREACH(inputs.input_array, input) {
    char address[NUM_TRYTES_ADDRESS + 1] = { 0 };
    flex_trits_to_trytes((tryte_t*)address, NUM_TRYTES_ADDRESS, input->address, NUM_TRITS_ADDRESS, NUM_TRITS_ADDRESS);
    cJSON* obj = cJSON_CreateObject();
    cJSON_AddStringToObject(obj, "address", address);
    char balance[64] = { 0 };
#ifdef WIN32
    snprintf(balance, 63, "%I64u", input->balance);
#else
    snprintf(balance, 63, "%llu", input->balance);
#endif
    cJSON_AddStringToObject(obj, "balance", balance);

    char key_index[64] = { 0 };
#ifdef WIN32
    snprintf(key_index, 63, "%I64u", input->key_index);
#else
    snprintf(key_index, 63, "%llu", input->key_index);
#endif
    cJSON_AddStringToObject(obj, "keyIndex", key_index);
    cJSON_AddItemToArray(json, obj);
  }
  inputs_clear(&inputs);
  free_iota_client(&serv);
  return json;
}