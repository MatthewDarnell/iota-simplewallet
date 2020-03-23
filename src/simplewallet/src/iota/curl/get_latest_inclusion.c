//
// Created by matth on 2/25/2020.
//

#include "../api.h"
#include "../../config/http.h"
#include "../../config/logger.h"

static cJSON* get_node_info() {
  cJSON* request = cJSON_CreateObject();
  cJSON_AddStringToObject(request, "command", "getNodeInfo");
  char* str_request = cJSON_PrintUnformatted(request);
  cJSON_Delete(request);
  cJSON* response = NULL;
  int ret;
  if((ret = post(str_request, &response)) < 0) {
    log_wallet_error("%s failed with %d\n", __func__, ret);
    cJSON_Delete(response);
    return NULL;
  }
  return response;
}


void get_latest_inclusion(cJSON** addresses_with_transactions, int include_unconfirmed) {
  int i,j;

  cJSON* hash_array = NULL;

  for(i=0; i < cJSON_GetArraySize(*addresses_with_transactions); i++) {
    cJSON* address_object = cJSON_GetArrayItem(*addresses_with_transactions, i);
    if(!cJSON_HasObjectItem(address_object, "transactions")) {
      continue;
    }

    cJSON* transaction_array = cJSON_GetObjectItem(address_object, "transactions");
    size_t num_txs = cJSON_GetArraySize(transaction_array);

    if(num_txs < 1) {
      continue;
    }


    cJSON* request = cJSON_CreateObject();
    cJSON_AddStringToObject(request, "command", "getInclusionStates");
    hash_array = cJSON_CreateArray();

    for(j = 0; j < num_txs; j++) {
      cJSON* obj = cJSON_GetArrayItem(transaction_array, j);
      const char* hash = cJSON_GetObjectItem(obj, "hash")->valuestring;
      cJSON_AddItemToArray(hash_array, cJSON_CreateString(hash));
    }

    cJSON_AddItemToObject(request, "transactions", hash_array);

    cJSON* info = get_node_info();

    if(!info) {
      log_wallet_error("%s unable to get node info\n", __func__);
      continue;
    }
    const char* milestone = cJSON_GetObjectItem(info, "latestSolidSubtangleMilestone")->valuestring;


    cJSON_AddItemToObject(request, "tips", cJSON_CreateStringArray(&milestone, 1));

    cJSON_Delete(info);

    char* str_request = cJSON_PrintUnformatted(request);
    cJSON_Delete(request);
    cJSON* response = NULL;

    int ret;
    if((ret = post(str_request, &response)) < 0) {
      log_wallet_error("%s failed with %d\n", __func__, ret);
      cJSON_Delete(response);
      continue;
    }

    if(!cJSON_HasObjectItem(response, "states")) {
      log_wallet_error("\nExiting with %s\n", cJSON_Print(response));
      cJSON_Delete(response);
      continue;
    }

    cJSON* states = cJSON_GetObjectItem(response, "states");

    for (j = 0; j < (int)cJSON_GetArraySize(states); j++) {
      cJSON* obj = cJSON_GetArrayItem(transaction_array, j);
      cJSON* state = cJSON_GetArrayItem(states, j);

      int confirmed =
          cJSON_IsTrue(state) ? 1 : 0;
      cJSON_AddStringToObject(obj, "confirmed", confirmed == 1? "true" : "false");
    }

    if(!include_unconfirmed) {
      j = 0;
      while(1) {
        if(j >=  cJSON_GetArraySize(transaction_array)) {
          break;
        }
        cJSON* obj = cJSON_GetArrayItem(transaction_array, j);
        char* c = cJSON_GetObjectItem(obj, "confirmed")->valuestring;
        if(strcasecmp(c, "false") == 0) {
          cJSON_DeleteItemFromArray(transaction_array, j);
          j--;
        }
        j++;
      }
    }
  }
}