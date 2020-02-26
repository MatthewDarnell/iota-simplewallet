//
// Created by matth on 2/24/2020.
//

#ifndef IOTA_SIMPLEWALLET_HTTP_H
#define IOTA_SIMPLEWALLET_HTTP_H
#include <curl/curl.h>
#include <cjson/cJSON.h>
int get(const char* url, cJSON** out);
int post(const char* query_string, cJSON** out);
#endif //IOTA_SIMPLEWALLET_HTTP_H
