//
// Created by matth on 2/19/2020.
//

#include <stdio.h>
#include <cclient/api/core/core_api.h>
#include <common/trinary/flex_trit.h>
#include "../../config/config.h"
#include "../../config/logger.h"
#include "../api.h"

void get_address_balance(cJSON** addresses, uint64_t min_iota, int remove_low_balance_addrs) {
  iota_client_service_t* serv = get_iota_client();
  get_balances_req_t *balance_req = get_balances_req_new();
  get_balances_res_t *balance_res = get_balances_res_new();

  if(!serv || !balance_req || !balance_res) {
    log_wallet_error( "%s oom\n", __func__);
    return;
  }

  int i;
  for(i=0; i < cJSON_GetArraySize(*addresses); i++) {
    flex_trit_t flex_addr[NUM_FLEX_TRITS_ADDRESS] = { 0 };
    const char* address = cJSON_GetObjectItem(cJSON_GetArrayItem(*addresses, i), "address")->valuestring;
    flex_trits_from_trytes(flex_addr, NUM_TRITS_ADDRESS, (tryte_t*)address, NUM_TRYTES_ADDRESS, NUM_TRYTES_ADDRESS);
    get_balances_req_address_add(balance_req, flex_addr);
    balance_req->threshold = 100;
  }

  retcode_t ret_code = RC_ERROR;
  if((ret_code = iota_client_get_balances(serv, balance_req, balance_res)) != RC_OK) {
    log_wallet_error( "Get balances returned %s\n", error_2_string(ret_code));
    get_balances_req_free(&balance_req);
    get_balances_res_free(&balance_res);
    iota_client_core_destroy(&serv);
    return;
  }

  size_t balance_cnt = get_balances_res_balances_num(balance_res);
  for(i=0; i < balance_cnt; i++) {
    uint64_t balance =  get_balances_res_balances_at(balance_res, i);

    if(balance < min_iota) {
      continue;
    }

    char str_balance[64] = { 0 };
#ifdef WIN32
    snprintf(str_balance, 64, "%I64u", balance);
#else
    snprintf(str_balance, 64, "%lld", balance);
#endif

    cJSON* address = cJSON_GetArrayItem(*addresses, i);
    if(cJSON_HasObjectItem(address, "balance")) {
      cJSON_DeleteItemFromObject(address, "balance");
    }
    cJSON_AddStringToObject(address, "balance", str_balance);
  }

  if(remove_low_balance_addrs > 0) {
    i = 0;
    while(1) {
      if(i >=  cJSON_GetArraySize(*addresses)) {
        break;
      }
      cJSON* obj = cJSON_GetArrayItem(*addresses, i);
      if(!cJSON_HasObjectItem(obj, "balance")) {
        cJSON_DeleteItemFromArray(*addresses, i);
        i--;
      } else {
        const char* balance = cJSON_GetObjectItem(obj, "balance")->valuestring;
        uint64_t d_balance = strtoull(balance, NULL, 10);
        if(d_balance < min_iota) {
          cJSON_DeleteItemFromArray(*addresses, i);
          i--;
        }
      }
      i++;
    }
  }



  get_balances_req_free(&balance_req);
  get_balances_res_free(&balance_res);
  iota_client_core_destroy(&serv);
}
