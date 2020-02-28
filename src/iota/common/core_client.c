//
// Created by matth on 2/22/2020.
//

#include <cjson/cJSON.h>
#include <cclient/api/core/core_api.h>
#include "../../config/logger.h"
#include "../../config/config.h"
#include "../api.h"

char *host = NULL,
     *pem = NULL;
int port;


void init_iota_client() {
  char* str_nodes = get_config("nodes");
  cJSON* nodes = cJSON_Parse(str_nodes);
  free(str_nodes);

  if(cJSON_GetArraySize(nodes) < 1) {
    log_wallet_error("Unable to get nodes\n", "");
    cJSON_Delete(nodes);
    return;
  }
  cJSON* node = cJSON_GetArrayItem(nodes, 0);

  host = strdup(cJSON_GetObjectItem(node, "host")->valuestring);
  pem = strdup(cJSON_GetObjectItem(node, "pem")->valuestring);
  port = strtol(cJSON_GetObjectItem(node, "port")->valuestring, NULL, 10);
  cJSON_Delete(nodes);
}

iota_client_service_t* get_iota_client() {
  if(!host || !pem) {
    init_iota_client();
  }
  return iota_client_core_init(host, port, pem);
}

void free_iota_client(iota_client_service_t** serv) {
  iota_client_core_destroy(serv);
}

void shutdown_iota_client() {
  free(host);
  free(pem);
}