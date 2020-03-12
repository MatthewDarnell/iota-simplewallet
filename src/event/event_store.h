//
// Created by Matthew Darnell on 3/11/20.
//

#ifndef IOTA_SIMPLEWALLET_EVENT_STORE_H
#define IOTA_SIMPLEWALLET_EVENT_STORE_H
int init_event_store();
void shutdown_event_store();

int is_valid_event(const char* event);
char* get_all_valid_events();
int register_new_callback(const char* event, void* (cb)(char*));
int fire_registered_callbacks(const char* event, char* value);
#endif //IOTA_SIMPLEWALLET_EVENT_STORE_H
