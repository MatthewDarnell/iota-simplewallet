//
// Created by matth on 2/16/2020.
//

#ifndef IOTA_SIMPLEWALLET_CONFIG_H
#define IOTA_SIMPLEWALLET_CONFIG_H
#include <stdint.h>
//Load  initial config  from  file path
//@path NULL for defaults, file path for  json  file
//returns 0 for success, -1 for  failure
int load_config(const char *path);

//Shutdown the config file
void shutdown_config();

//Get Current  Configuration Value
//returns NULL if not found, otherwise  remember to free!
char* get_config(const char *key);

//Set  Configuration Value
//@save 1: save to file, otherwise only set  in-memory
int set_config(const char* key, const char* value, int8_t save);
#endif //IOTA_SIMPLEWALLET_CONFIG_H
