//
// Created by matth on 2/29/2020.
//

#include <pthread.h>
#include "../deposit_detector.h"
#include "start.h"

int quit_flag = 0;
pthread_t threads[1];


void start_threads() {
  pthread_create(&(threads[0]), NULL, (void*)&thread_deposit_detector, (void*)&quit_flag);
}

void join_threads() {
  pthread_join(threads[0], NULL);
}
void shutdown_threads() {
  quit_flag = 1;
}
