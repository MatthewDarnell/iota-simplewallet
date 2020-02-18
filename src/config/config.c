//
// Created by matth on 2/16/2020.
//
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <cjson/cJSON.h>
#include "config.h"
cJSON *config = NULL;
const char *default_config = "{"
                        "\"database\": \"iota-simplewallet.db\","
                        "\"minAddressPool\": \"5\""
                        "}";
const char* default_path = "wallet.conf";

int __is_valid_config(cJSON* object) {  //object exists with the minimal set of parameters
  if(!object) return -1;
  if(!cJSON_HasObjectItem(object, "database")) return -2;
  if(!cJSON_HasObjectItem(object, "minAddressPool")) return -3;
  return 0;
}

//Check whether a file exists and is not 0 length
//@return 0 if yes, < 0 if no
int __does_file_exist(const char* path) {
  FILE *iFile = fopen(path, "rb");
  if(!iFile) {
    return -1;
  }
  long length;
  fseek(iFile, 0, SEEK_END);
  length = ftell(iFile);
  fclose(iFile);
  return (length > 1) ? 0 : -2;
}

//Read a file into a cJSON struct
cJSON* __read_file(const char* path) {

  FILE* iFile = fopen(path, "rb");
  if(!iFile) {
    fprintf(stderr, "Unable to open configuration file <%s>\n", path);
    return NULL;
  }

  fseek(iFile, 0, SEEK_END);
  long length = ftell(iFile);
  fseek(iFile, 0, SEEK_SET);

  char* buffer = (char*) calloc(sizeof(char), length+1);
  if(!buffer) {
    fprintf(stderr, "Unable to allocate memory for config file <%s>\n", path);
    return NULL;
  }

  size_t read_bytes = fread(buffer, 1, length, iFile);
  fclose(iFile);
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
  FILE *oFile = fopen(path, "wb+");
  if(!oFile) {
    fprintf(stderr, "Error creating config file %s --- Bailing out!\n", path);
    exit(1);
  }
  fprintf(oFile, "%s", data);
  fclose(oFile);
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
    cJSON_AddStringToObject(config, "path", path);
  }

  const char* config_file = cJSON_GetObjectItem(config, "path")->valuestring;
  int file_exists = __does_file_exist(config_file);

  if(file_exists == 0) {  //File exists and is is not empty, let's read it
    cJSON* config_obj = __read_file(config_file);
    if(!config_obj) { //Invalid Json, just load default settings
      printf("Loading Default configuration settings\n");
      cJSON_AddItemToObject(
        config,
        "config",
        cJSON_Parse(default_config)
      );
    } else {
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
  __write_to_file_and_create(config_file, default_config);
  cJSON_AddItemToObject(
    config,
    "config",
    cJSON_Parse(default_config)
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
  char *strValue = strdup(
      cJSON_GetStringValue(value)
    );
  return  strValue;
}

//Set  Configuration Value
//@save 1: save to file, otherwise only set  in-memory
int set_config(const char  *key, const char *value, int8_t save) {
  if(!config) return -1;
  if(!cJSON_HasObjectItem(config, "config")) return -1;
  cJSON *config_obj = cJSON_GetObjectItem(config, "config");
  if(cJSON_HasObjectItem(config_obj, key)) {
    cJSON_Delete(
        cJSON_GetObjectItem(config_obj, key)
      );
  }
  cJSON_AddItemToObject(
    cJSON_GetObjectItem(config, "config"),
    key,
    cJSON_CreateString(value));
  if(save) {
    if(!cJSON_HasObjectItem(config, "path")) return -1;
    cJSON *path_obj = cJSON_GetObjectItem(config, "path");
    const char* path = path_obj->valuestring;

    char* buffer = cJSON_Print(
      cJSON_GetObjectItem(config, "config")
      );
    FILE *oFile = fopen(path, "wb");
    if (oFile == NULL){
      fprintf(stderr, "Error opening file for writing configuration.\n");
      return 1;
    }
    fprintf(oFile, "%s", buffer);
    free(buffer);
    fclose(oFile);
  }
  return  0;
}