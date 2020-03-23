//
// Created by matth on 2/22/2020.
//

#include <string.h>
#include <cjson/cJSON.h>
#include <cclient/api/core/core_api.h>
#include "../../iota-simplewallet.h"
#include "../api.h"

char *_host = NULL,
     *_pem = NULL;
int _port;


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

  _host = strdup(cJSON_GetObjectItem(node, "host")->valuestring);
  _pem = strdup(cJSON_GetObjectItem(node, "pem")->valuestring);
  _port = strtol(cJSON_GetObjectItem(node, "port")->valuestring, NULL, 10);
  cJSON_Delete(nodes);
}

int set_node(char* host, int port) {
  if(_host) {
    free(_host);
  }
  _host = strdup(host);
  _port = port;
  return 0;
}

cJSON* get_node() {
  cJSON* json = cJSON_CreateObject();
  cJSON_AddStringToObject(json, "host", _host);
  cJSON_AddNumberToObject(json, "port", _port);
  return json;
}

iota_client_service_t* get_iota_client() {
  if(!_host || !_pem) {
    init_iota_client();
  }
  return iota_client_core_init(_host, _port, _pem);
}

void free_iota_client(iota_client_service_t** serv) {
  iota_client_core_destroy(serv);
}

void shutdown_iota_client() {
  free(_host);
  free(_pem);
}