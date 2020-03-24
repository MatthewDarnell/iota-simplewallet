//
// Created by matth on 2/16/2020.
//
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <cjson/cJSON.h>
#include "../iota-simplewallet.h"
pthread_mutex_t config_file_mutex = PTHREAD_MUTEX_INITIALIZER;
cJSON *config = NULL;
const char *default_config = "{"
                        "\"minAddressPool\": \"5\","
                        "\"minAddressesToCheckWhenSyncing\": \"25\","
                        "\"nodes\": ["
                        "{"
                        "\"pem\": \"-----BEGIN CERTIFICATE-----\r\nMIIDQTCCAimgAwIBAgITBmyfz5m/jAo54vB4ikPmljZbyjANBgkqhkiG9w0BAQsF\r\nADA5MQswCQYDVQQGEwJVUzEPMA0GA1UEChMGQW1hem9uMRkwFwYDVQQDExBBbWF6\r\nb24gUm9vdCBDQSAxMB4XDTE1MDUyNjAwMDAwMFoXDTM4MDExNzAwMDAwMFowOTEL\r\nMAkGA1UEBhMCVVMxDzANBgNVBAoTBkFtYXpvbjEZMBcGA1UEAxMQQW1hem9uIFJv\r\nb3QgQ0EgMTCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBALJ4gHHKeNXj\r\nca9HgFB0fW7Y14h29Jlo91ghYPl0hAEvrAIthtOgQ3pOsqTQNroBvo3bSMgHFzZM\r\n9O6II8c+6zf1tRn4SWiw3te5djgdYZ6k/oI2peVKVuRF4fn9tBb6dNqcmzU5L/qw\r\nIFAGbHrQgLKm+a/sRxmPUDgH3KKHOVj4utWp+UhnMJbulHheb4mjUcAwhmahRWa6\r\nVOujw5H5SNz/0egwLX0tdHA114gk957EWW67c4cX8jJGKLhD+rcdqsq08p8kDi1L\r\n93FcXmn/6pUCyziKrlA4b9v7LWIbxcceVOF34GfID5yHI9Y/QCB/IIDEgEw+OyQm\r\njgSubJrIqg0CAwEAAaNCMEAwDwYDVR0TAQH/BAUwAwEB/zAOBgNVHQ8BAf8EBAMC\r\nAYYwHQYDVR0OBBYEFIQYzIU07LwMlJQuCFmcx7IQTgoIMA0GCSqGSIb3DQEBCwUA\r\nA4IBAQCY8jdaQZChGsV2USggNiMOruYou6r4lK5IpDB/G/wkjUu0yKGX9rbxenDI\r\nU5PMCCjjmCXPI6T53iHTfIUJrU6adTrCC2qJeHZERxhlbI1Bjjt/msv0tadQ1wUs\r\nN+gDS63pYaACbvXy8MWy7Vu33PqUXHeeE6V/Uq2V8viTO96LXFvKWlJbYK8U90vv\r\no/ufQJVtMVT8QtPHRh8jrdkPSHCa2XV4cdFyQzR1bldZwgJcJmApzyMZFo6IQ6XU\r\n5MsI+yMRQ+hDKXJioaldXgjUkK642M4UwtBV8ob2xJNDd2ZhwLnoQdeXeGADbkpy\r\nrqXRfboQnoZsG4q5WTP468SQvvG5\r\n-----END CERTIFICATE-----\r\n\","
                        "\"host\": \"nodes.thetangle.org\","
                        "\"port\": \"443\""
                        "}"
                        "],"
                        "\"sendTag\": \"IOTA9C9WALLET\","
                        "\"events\": ["
                        "\"node_updated\","
                        "\"balance_changed\","
                        "\"transaction_received\","
                        "\"transaction_received_confirmed\","
                        "\"transaction_sent\","
                        "\"sent_transaction_confirmed\""
                        "]"
                        "}";
const char* default_log = "debug.log";
const char* default_path = "iota.conf";
const char* default_db = "wallet.db";

//Returns number of overwritten configs
int __fill_missing_configs_with_defaults(cJSON** object) {  //object exists with the minimal set of parameters
  cJSON* default_json = cJSON_Parse(default_config);
  cJSON* temp_key = NULL;
  int num_overwrites = 0;
  cJSON_ArrayForEach(temp_key, default_json) {
    if(!cJSON_HasObjectItem(*object, temp_key->string)) {
      cJSON* temp_value = cJSON_DetachItemFromObject(default_json, temp_key->string);
      cJSON_AddItemToObject(*object, temp_key->string, temp_value);
      num_overwrites++;
    } else {
    }
  }
  cJSON_Delete(default_json);
  return num_overwrites;
}

//Check whether a file exists and is not 0 length
//@return 0 if yes, < 0 if no
int __does_file_exist(const char* path) {
  pthread_mutex_lock(&config_file_mutex);
  FILE *iFile = fopen(path, "rb");
  if(!iFile) {
    pthread_mutex_unlock(&config_file_mutex);
    return -1;
  }
  long length;
  fseek(iFile, 0, SEEK_END);
  length = ftell(iFile);
  fclose(iFile);
  pthread_mutex_unlock(&config_file_mutex);
  return (length > 1) ? 0 : -2;
}

//Read a file into a cJSON struct
cJSON* __read_file(const char* path) {
  pthread_mutex_lock(&config_file_mutex);
  FILE* iFile = fopen(path, "rb");
  if(!iFile) {
    pthread_mutex_unlock(&config_file_mutex);
    fprintf(stderr, "Unable to open configuration file <%s>\n", path);
    return NULL;
  }

  fseek(iFile, 0, SEEK_END);
  long length = ftell(iFile);
  fseek(iFile, 0, SEEK_SET);

  char* buffer = (char*) calloc(sizeof(char), length+1);
  if(!buffer) {
    pthread_mutex_unlock(&config_file_mutex);
    fprintf(stderr, "Unable to allocate memory for config file <%s>\n", path);
    return NULL;
  }

  size_t read_bytes = fread(buffer, 1, length, iFile);
  fclose(iFile);
  pthread_mutex_unlock(&config_file_mutex);
  if(read_bytes != length) {
    fprintf(stderr, "Unable to read config file <%s>\n", path);
    return NULL;
  }

  cJSON* json = cJSON_Parse(buffer);
  free(buffer);

  if(!json) {
    fprintf(stderr, "Unable to read config file <%s> --- Is it valid Json?\n", path);
    return NULL;
  }
  return json;
}

int __write_to_file_and_create(const char* path, const char* data) {
  pthread_mutex_lock(&config_file_mutex);
  FILE *oFile = fopen(path, "wb+");
  if(!oFile) {
    pthread_mutex_unlock(&config_file_mutex);
    fprintf(stderr, "Error creating config file %s --- Bailing out!\n", path);
    exit(1);
  }
  fprintf(oFile, "%s", data);
  fclose(oFile);
  pthread_mutex_unlock(&config_file_mutex);
  return 0;
}


//Load  initial config  from  file path
//@path NULL for defaults, file path for  json  file
//returns 0 for success, -1 for  failure
int load_config(const char *path) {
  if(config) {
    cJSON_Delete(config);
  }
  config = cJSON_CreateObject();

  if(!path) {
    cJSON_AddStringToObject(config, "path", default_path);
    printf("Loading default configuration file at <%s>\n", default_path);
  } else {
    size_t path_size = strlen(path) + strlen(default_path) + 3;
    char config_file[path_size];
    memset(config_file, 0, path_size);
    snprintf(config_file, path_size-1, "%s/%s", path, default_path);
    printf("Loading default configuration file at <%s>\n", config_file);
    cJSON_AddStringToObject(config, "path", config_file);
  }

  const char* config_file = cJSON_GetObjectItem(config, "path")->valuestring;
  int file_exists = __does_file_exist(config_file);

  size_t log_size = strlen(path) + strlen(default_log) + 3;
  char log_file[log_size];
  memset(log_file, 0, log_size);
  snprintf(log_file, log_size-1, "%s/%s", path, default_log);

  size_t db_size = strlen(path) + strlen(default_db) + 3;
  char db_file[db_size];
  memset(db_file, 0, db_size);
  snprintf(db_file, db_size-1, "%s/%s", path, default_db);

  if(file_exists == 0) {  //File exists and is is not empty, let's read it
    cJSON* config_obj = __read_file(config_file);
    if(!config_obj) { //Invalid Json, just load default settings
      printf("Loading Default configuration settings\n");
      cJSON* c = cJSON_Parse(default_config);
      cJSON_AddStringToObject(c, "logFile", log_file);
      cJSON_AddStringToObject(c, "database", db_file);
      cJSON_AddItemToObject(
        config,
        "config",
        c
      );
    } else {

      if(__fill_missing_configs_with_defaults(&config_obj) > 0) {
        char* data_to_overwrite = cJSON_Print(config_obj);
        __write_to_file_and_create(config_file, data_to_overwrite);
        free(data_to_overwrite);
      }

      cJSON_AddItemToObject(
          config,
          "config",
          config_obj
        );
    }
    return 0;
  }

  //Config file does not exist
  printf("Configuration file not found. Creating <%s> with default settings\n", config_file);
  cJSON* c = cJSON_Parse(default_config);
  cJSON_AddStringToObject(c, "logFile", log_file);
  cJSON_AddStringToObject(c, "database", db_file);
  char* str_config = cJSON_Print(c);
  __write_to_file_and_create(config_file, str_config);
  free(str_config);
  cJSON_AddItemToObject(
    config,
    "config",
    c
  );
  return 0;
}

//Shutdown the config file
void shutdown_config()  {
  if(config) {
    cJSON_Delete(config);
  }
}

//Get Current  Configuration Value
//returns NULL if not found, otherwise  remember to free!
char* get_config(const char *key) {
  if(!config) return NULL;
  if(!cJSON_HasObjectItem(config, "config")) return NULL;
  cJSON *config_obj = cJSON_GetObjectItem(config, "config");
  cJSON *value = cJSON_GetObjectItem(config_obj, key);

  char *strValue = NULL;
  if(cJSON_GetStringValue(value) != NULL) {
    strValue = strdup(
      cJSON_GetStringValue(value)
    );
  } else {  //some config options aren't strings, but json
    strValue = cJSON_Print(value);
  }

  return  strValue;
}

//Set  Configuration Value
//@save 1: save to file, otherwise only set  in-memory
int set_config(const char* key, const char* value, int8_t save) {
  pthread_mutex_lock(&config_file_mutex);
  if(!config) {
    pthread_mutex_unlock(&config_file_mutex);
    return -1;
  }
  if(!cJSON_HasObjectItem(config, "config")) {
    pthread_mutex_unlock(&config_file_mutex);
    return -1;
  }
  cJSON *config_obj = cJSON_GetObjectItem(config, "config");
  if(cJSON_HasObjectItem(config_obj, key)) {
    cJSON_DeleteItemFromObject(config_obj, key);
  }

  cJSON_AddStringToObject(
    config_obj,
    key,
    value);
  if(save) {
    if(!cJSON_HasObjectItem(config, "path")) return -1;
    cJSON *path_obj = cJSON_GetObjectItem(config, "path");
    const char* path = path_obj->valuestring;

    char* buffer = cJSON_Print(
      cJSON_GetObjectItem(config, "config")
      );

    FILE *oFile = fopen(path, "wb");
    if (oFile == NULL){
      pthread_mutex_unlock(&config_file_mutex);
      fprintf(stderr, "Error opening file for writing configuration.\n");
      return 1;
    }
    fprintf(oFile, "%s", buffer);
    free(buffer);
    fclose(oFile);
  }
  pthread_mutex_unlock(&config_file_mutex);
  return  0;
}