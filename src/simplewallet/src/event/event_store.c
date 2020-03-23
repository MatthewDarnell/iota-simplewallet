//
// Created by Matthew Darnell on 3/11/20.
//

#include <stdlib.h>
#include <string.h>
#include <cjson/cJSON.h>
#include <utarray.h>
#include "../iota-simplewallet.h"
#include "event_store.h"

typedef struct {
    char event_type[32];
    UT_array* callbacks_to_fire;
} callback_events;

typedef struct callback {
    void* (*cb)(const char*);
} cb;

UT_icd cb_icd = {sizeof(cb), NULL, NULL, NULL};


static callback_events* events_to_fire = NULL;
static cJSON* valid_events = NULL;

int init_event_store() {
  char *events = get_config("events");
  if (!events) {
    log_wallet_error("%s unable to find events from config", __func__)
    return -1;
  }

  valid_events = cJSON_Parse(events);
  free(events);

  if(!valid_events) {
    log_wallet_error("%s unable to parse events from config", __func__)
    return -1;
  }

  if(!cJSON_IsArray(valid_events)) {
    cJSON_Delete(valid_events);
    log_wallet_error("%s events are invalid array", __func__)
    return -1;
  }
  cJSON* event = NULL;

  int num_event_types = cJSON_GetArraySize(valid_events);
  events_to_fire = malloc(sizeof(callback_events) * num_event_types);

  if(!events_to_fire) {
    cJSON_Delete(valid_events);
    log_wallet_error("%s Unable to setup events (OOM)", __func__)
    return -1;
  }

  int i = 0;
  cJSON_ArrayForEach(event, valid_events) {
    const char* event_type = cJSON_GetStringValue(event);
    memset(events_to_fire[i].event_type, 0, 32);
    strncpy(events_to_fire[i].event_type, event_type, 31);
    utarray_new(events_to_fire[i].callbacks_to_fire, &cb_icd);
    i++;
  }

  return 0;
}

void shutdown_event_store() {
  if(valid_events) {
    cJSON* event = NULL;
    int i = 0;
    cJSON_ArrayForEach(event, valid_events) {
      utarray_free(events_to_fire[i].callbacks_to_fire);
      i++;
    }
    free(events_to_fire);
    cJSON_Delete(valid_events);
  }

}

int is_valid_event(const char* event) {
  cJSON* json_event = NULL;
  int event_found = 0;
  cJSON_ArrayForEach(json_event, valid_events) {
    char* e = cJSON_GetStringValue(json_event);
    if(strcasecmp(event, e) == 0) {
      event_found = 1;
      break;
    }
  }
  if(event_found < 1) {
    return -1;
  }
  return 0;
}

char* get_all_valid_events() {
  return cJSON_Print(valid_events);
}


int register_new_callback(const char* event, void* (*new_callback)(const char*)) {
  cJSON* json_event = NULL;
  int event_found = 0;
  int i = 0;
  cJSON_ArrayForEach(json_event, valid_events) {
    char* e = cJSON_GetStringValue(json_event);
    if(strcasecmp(event, e) == 0) {
      cb callback = { .cb=new_callback };
      utarray_push_back(events_to_fire[i].callbacks_to_fire, &callback);
      event_found = 1;
      break;
    }
    i++;
  }
  if(event_found < 1) {
    log_wallet_error("%s event <%s> not found", __func__, event);
    return -1;
  }

  return 0;
}

int fire_registered_callbacks(const char* event, const char* value) {
  cJSON* json_event = NULL;
  int event_found = 0;
  int i = 0;
  cJSON_ArrayForEach(json_event, valid_events) {
    char* e = cJSON_GetStringValue(json_event);
    if(strcasecmp(event, e) == 0) {
      UT_array* callback_array_to_fire = events_to_fire[i].callbacks_to_fire;
      cb* p = NULL;
      while( (p =(cb*)utarray_next(callback_array_to_fire, p))) {
        p->cb(value);
      }
      event_found = 1;
      break;
    }
    i++;
  }
  if(event_found < 1) {
    log_wallet_error("%s event <%s> not found", __func__, event);
    return -1;
  }
  return 0;
}