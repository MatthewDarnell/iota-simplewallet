//
// Created by matth on 3/10/2020.
//

#include <stdlib.h>
#include <cjson/cJSON.h>
#include "../iota/api.h"
#include "../iota-simplewallet.h"

char* get_node_status() {
  cJSON* json = NULL;
  char* info = get_config("info");
  if(!info) {
    return NULL;
  }
  json = cJSON_Parse(info);
  free(info);

  cJSON* node = get_node();
  cJSON_AddItemToObject(json, "node", node);
  char* ret_val = cJSON_Print(json);
  cJSON_Delete(json);
  return ret_val;
}