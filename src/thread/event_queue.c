//
// Created by Matthew Darnell on 3/11/20.
//

#ifdef WIN32
#include <windows.h>
#else
#include <unistd.h>
#define Sleep(x) sleep(x/1000)
#endif

#include <stdio.h>
#include <pthread.h>
#include <cjson/cJSON.h>
#include <utarray.h>
#include "../iota-simplewallet.h"
#include "../event/event_store.h"
#include "event_queue.h"

UT_array* loaded_events = NULL;

void push_new_event(char* event, char* value) {
  if(!loaded_events) {
    log_wallet_debug(" Trying to push event before setup", "");
    return;
  }

  cJSON* json_value = cJSON_CreateObject();
  cJSON_AddStringToObject(json_value, "event", event);
  cJSON_AddStringToObject(json_value, "value", value);
  char* json = cJSON_PrintUnformatted(json_value);
  utarray_push_back(loaded_events, &json);
  free(json);
  cJSON_Delete(json_value);
}


void thread_event_queue(void* args) {
  log_wallet_info("Starting Event Queue Thread", "");
  utarray_new(loaded_events, &ut_str_icd);

  char**  p = NULL;

  int* quit_flag = (int*)args;

  while(1) {
    if (*quit_flag != 0) {
      break;
    }

    Sleep(100);

    while ((p = (char**)utarray_next(loaded_events,p))) {
      cJSON* event = NULL;
      log_wallet_debug("%s\n",*p);
      event = cJSON_Parse(*p);
      if(!event) {
        continue;
      }

      if(!cJSON_HasObjectItem(event, "event") || !cJSON_HasObjectItem(event, "value")) {
        cJSON_Delete(event);
        continue;
      }
      char* event_type = cJSON_GetObjectItem(event, "event")->valuestring;
      char* value_to_fire = cJSON_GetObjectItem(event, "value")->valuestring;
      log_wallet_debug("Firing <%s> event", event_type);
      fire_registered_callbacks(event_type, value_to_fire);
      cJSON_Delete(event);
    }
    utarray_clear(loaded_events);

  }
  utarray_free(loaded_events);
  log_wallet_info("Shutting Down Event Queue Thread", "");
  pthread_exit(0);
}