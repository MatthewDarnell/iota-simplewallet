//
// Created by matth on 3/8/2020.
//

#include <stdio.h>
#include <cclient/api/core/core_api.h>
#include <common/trinary/flex_trit.h>
#include "../../config/config.h"
#include "../../config/logger.h"
#include "../api.h"

void were_addresses_spent_from(cJSON** addresses) {
  iota_client_service_t *serv = get_iota_client();
  were_addresses_spent_from_req_t *addr_spent_req = were_addresses_spent_from_req_new();
  were_addresses_spent_from_res_t *addr_spent_res = were_addresses_spent_from_res_new();

  if (!serv || !addr_spent_req || !addr_spent_res) {
    log_wallet_error( "%s oom\n", __func__);
    return;
  }

  flex_trit_t trits_243[FLEX_TRIT_SIZE_243];
  retcode_t ret_code = RC_OK;

  cJSON* address = NULL;
  cJSON_ArrayForEach(address, *addresses) {
    char* addr = cJSON_GetObjectItem(address, "address")->valuestring;
    flex_trits_from_trytes(trits_243, NUM_TRITS_HASH, (tryte_t*)addr, NUM_TRYTES_HASH, NUM_TRYTES_HASH);
    if (were_addresses_spent_from_req_add(addr_spent_req, trits_243) != RC_OK) {
      log_wallet_error("%s: adding address failed. (%s)\n", __func__, addr);
    }
  }

  if ((ret_code = iota_client_were_addresses_spent_from(serv, addr_spent_req, addr_spent_res)) == RC_OK) {
    for (size_t i = 0; i < were_addresses_spent_from_res_states_count(addr_spent_res); i++) {
      cJSON* obj = cJSON_GetArrayItem(*addresses, i);
      cJSON_AddNumberToObject(
        obj,
        "spent_from",
        were_addresses_spent_from_res_states_at(addr_spent_res, i) ? 1 : 0
        );
    }
  } else {
    log_wallet_error("%s: %s\n", __func__, error_2_string(ret_code));
  }

  were_addresses_spent_from_req_free(&addr_spent_req);
  were_addresses_spent_from_res_free(&addr_spent_res);
  iota_client_core_destroy(&serv);

}