//
// Created by matth on 2/25/2020.
//

#include <stdio.h>
#include "../api.h"
#include "../../config/logger.h"
#include "../../config/config.h"
#include "../../config/http.h"

void get_address_balance(cJSON** addresses, uint64_t min_iota) {
  printf("%s\n", __func__);
  cJSON* request = cJSON_CreateObject();
  cJSON* response = NULL;

  cJSON_AddStringToObject(request, "command", "getBalances");
  cJSON_AddNumberToObject(request, "threshold", 100);
  cJSON_AddItemToObject(request, "addresses", cJSON_CreateArray());
  cJSON* address_array = cJSON_GetObjectItem(request, "addresses");

  int i;
  for (i = 0; i < cJSON_GetArraySize(*addresses); i++) {
    const char *address = cJSON_GetObjectItem(cJSON_GetArrayItem(*addresses, i), "address")->valuestring;
    cJSON_AddItemToArray(address_array, cJSON_CreateString(address));
  }

  char* request_string = cJSON_PrintUnformatted(request);
  cJSON_Delete(request);
  int p_ret = post(request_string, &response);
  free(request_string);

  if(!response) {
      log_wallet_error("Get Address Balance http request returned %d", p_ret);
      return;
  }

  if(!cJSON_HasObjectItem(response, "balances")) {
    cJSON_Delete(response);
    return;
  }
  cJSON* balances = cJSON_GetObjectItem(response, "balances");

  for(i = 0; i < cJSON_GetArraySize(balances); i++) {
    const char* str_balance = cJSON_GetArrayItem(balances, i)->valuestring;
    uint64_t balance = strtoull(str_balance, NULL, 10);
    if(balance < min_iota) {
      continue;
    }
    cJSON *address = cJSON_GetArrayItem(*addresses, i);
    if(cJSON_HasObjectItem(address, "balance")) {
      cJSON_DeleteItemFromObject(address, "balance");
    }
    cJSON_AddStringToObject(address, "balance", str_balance);
  }
  for(i=0; i<cJSON_GetArraySize(*addresses); i++) {
    cJSON* obj  =  cJSON_GetArrayItem(*addresses, i);
    if(!cJSON_HasObjectItem(obj, "balance")) {
      printf("removing addr %s\n", cJSON_GetObjectItem(obj,  "address")->valuestring);
      cJSON_Delete(obj);
      i--;
    }
  }
}