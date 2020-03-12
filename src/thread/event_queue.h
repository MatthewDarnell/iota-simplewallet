//
// Created by Matthew Darnell on 3/11/20.
//

#ifndef IOTA_SIMPLEWALLET_EVENT_QUEUE_H
#define IOTA_SIMPLEWALLET_EVENT_QUEUE_H
void thread_event_queue(void* args);
void push_new_event(char* event, char* value);
#endif //IOTA_SIMPLEWALLET_EVENT_QUEUE_H
