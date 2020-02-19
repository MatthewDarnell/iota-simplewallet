//
// Created by matth on 2/19/2020.
//

#include <stdio.h>
#include <stdint.h>
#include <cclient/api/core/core_api.h>
#include <cclient/api/extended/extended_api.h>
#include <common/trinary/flex_trit.h>
#include "../config/config.h"
#include "get_address_balance.h"

cJSON* get_address_balance(cJSON* addresses) {
  cJSON* balances = NULL;
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

  balances = cJSON_CreateArray();

  int i;
  for(i=0; i < cJSON_GetArraySize(addresses); i++) {
    get_balances_req_t *balance_req = get_balances_req_new();
    get_balances_res_t *balance_res = get_balances_res_new();

    flex_trit_t flex_addr[NUM_FLEX_TRITS_ADDRESS] = { 0 };
    const char* address = cJSON_GetObjectItem(cJSON_GetArrayItem(addresses, i), "address")->valuestring;
    flex_trits_from_trytes(flex_addr, NUM_TRITS_ADDRESS, (tryte_t*)address, NUM_TRYTES_ADDRESS, NUM_TRYTES_ADDRESS);
    get_balances_req_address_add(balance_req, flex_addr);
    balance_req->threshold = 100;
    iota_client_get_balances(serv, balance_req, balance_res);
    int64_t balance = get_balances_res_balances_num(balance_res);

    char str_balance[64] = { 0 };
#ifdef WIN32
    snprintf(str_balance, 64, "%I64u", balance);
#else
    snprintf(str_balance, 64, "%lld", balance);
#endif
    cJSON* balance_object = cJSON_CreateObject();
    cJSON_AddStringToObject(balance_object, "address", address);
    cJSON_AddStringToObject(balance_object, "balance", str_balance);

    cJSON_AddItemToArray(balances, balance_object);

    get_balances_req_free(&balance_req);
    get_balances_res_free(&balance_res);
  }
  return balances;
}