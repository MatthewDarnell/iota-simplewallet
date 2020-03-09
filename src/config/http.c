//
// Created by matth on 2/24/2020.
//

#include <stdio.h>
#include <string.h>
#include "logger.h"
#include "http.h"
#include "config.h"


struct memory_struct {
    char* memory;
    size_t size;
};

static size_t write_mem_callback(void* contents, size_t size, size_t nmemb, void* userp) {
  size_t real_size = size * nmemb;
  struct memory_struct* mem = (struct memory_struct*)userp;
  char* tmp = realloc(mem->memory, mem->size + real_size + 1);

  if(!tmp) {
    log_wallet_error("Not enough memory for http call, realloc returned NULL", "");
    return 0;
  }

  mem->memory = tmp;
  memcpy(
        &(mem->memory[mem->size]),
        contents,
        real_size
    );
  mem->size += real_size;
  mem->memory[mem->size] = 0;
  return real_size;
}



int get(const char* url, cJSON** out) {
  CURL *curl;
  CURLcode res;
  CURLcode* pres = &res;
  curl = curl_easy_init();
  if(!curl) {
    return -1;
  }
  struct memory_struct chunk;
  chunk.memory = malloc(1);
  chunk.size = 0;

  curl_easy_setopt(curl, CURLOPT_URL, url);
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_mem_callback);
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void*)&chunk);
  curl_easy_setopt(curl, CURLOPT_REFERER, "iota-simplewallet");
  curl_easy_setopt(curl, CURLOPT_USERAGENT, "libcurl-agent/1.0");


  struct curl_slist *headers = NULL;
  headers = curl_slist_append(headers, "Content-Type: application/json");
  headers = curl_slist_append(headers, "X-IOTA-API-Version: 1");
  curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

  /* Perform the request, res will get the return code */
  *pres = curl_easy_perform(curl);
  /* Check for errors */
  if(res != CURLE_OK) {
    log_wallet_error("curl_easy_perform() failed: %s\n",
            curl_easy_strerror(res));
  } else {
    *out = cJSON_Parse(chunk.memory);
  }

  free(chunk.memory);
  /* always cleanup */
  curl_easy_cleanup(curl);
  return res;
}


int post(const char* query_string, cJSON** out) {

  char* str_nodes = get_config("nodes");
  cJSON* nodes = cJSON_Parse(str_nodes);
  free(str_nodes);

  if(cJSON_GetArraySize(nodes) < 1) {
    fprintf(stderr, "Unable to get nodes\n");
    cJSON_Delete(nodes);
    return -1;
  }
  cJSON* node = cJSON_GetArrayItem(nodes, 0);

  char* host = cJSON_GetObjectItem(node, "host")->valuestring;
  char* port = cJSON_GetObjectItem(node, "port")->valuestring;

  char url[256] = { 0 };
  snprintf(url, 256, "https://%s:%s", host, port);
  cJSON_Delete(nodes);

  CURL *curl;
  CURLcode res;
  CURLcode* pres = &res;
  curl = curl_easy_init();
  if(!curl) {
    return -1;
  }
  struct memory_struct chunk;
  chunk.memory = malloc(1);
  chunk.size = 0;

  curl_easy_setopt(curl, CURLOPT_URL, url);
  curl_easy_setopt(curl, CURLOPT_POSTFIELDS, query_string);
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_mem_callback);
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void*)&chunk);
  curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
  curl_easy_setopt(curl, CURLOPT_REFERER, "iota-simplewallet");
  curl_easy_setopt(curl, CURLOPT_USERAGENT, "libcurl-agent/1.0");



  struct curl_slist *headers = NULL;
  headers = curl_slist_append(headers, "Content-Type: application/json");
  headers = curl_slist_append(headers, "X-IOTA-API-Version: 1");
  curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);


  /* Perform the request, res will get the return code */
  *pres = curl_easy_perform(curl);
  /* Check for errors */
  if(res != CURLE_OK) {
    log_wallet_error("curl_easy_perform() failed: %s\n",
              curl_easy_strerror(res));
  } else {
    *out = cJSON_Parse(chunk.memory);
  }
  free(chunk.memory);
  /* always cleanup */
  curl_easy_cleanup(curl);
  return res;
}
