//
// Created by Matthew Darnell on 3/11/20.
//

#include <string.h>
#include <stdlib.h>
#include <cjson/cJSON.h>
#include "../event/event_store.h"
#include "../iota-simplewallet.h"

int register_callback(const char* event, void* (*cb)(char*)) {
  if(is_valid_event(event) < 0) {
    log_wallet_error("%s: Invalid event type <%s>", __func__, event);
    return -1;
  }
  register_new_callback(event, cb);
  return 0;
}

char* get_valid_events() {
  return get_all_valid_events();
}

int init_events() {
  return init_event_store();
}

void shutdown_events() {
  return shutdown_event_store();
}