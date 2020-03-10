//
// Created by matth on 2/29/2020.
//

#include <pthread.h>
#include "../address_balance_detector.h"
#include "../deposit_detector.h"
#include "../send_transaction.h"
#include "../spent_from_detector.h"
#include "start.h"

int quit_flag = 0;
pthread_t threads[4];


void start_threads() {
  pthread_create(&(threads[0]), NULL, (void*)&thread_deposit_detector, (void*)&quit_flag);
  pthread_create(&(threads[1]), NULL, (void*)&thread_send_transaction, (void*)&quit_flag);
  pthread_create(&(threads[2]), NULL, (void*)&thread_spent_from_detector, (void*)&quit_flag);
  pthread_create(&(threads[3]), NULL, (void*)&thread_address_balance_detector, (void*)&quit_flag);
}

void join_threads() {
  pthread_join(threads[0], NULL);
  pthread_join(threads[1], NULL);
  pthread_join(threads[2], NULL);
  pthread_join(threads[3], NULL);
}
void shutdown_threads() {
  quit_flag = 1;
}
