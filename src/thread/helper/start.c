//
// Created by matth on 2/29/2020.
//

#include <pthread.h>
#include "../address_balance_detector.h"
#include "../deposit_detector.h"
#include "../event_queue.h"
#include "../node_info_updater.h"
#include "../send_transaction.h"
#include "../sent_transaction_confirmer.h"
#include "../spent_from_detector.h"
#include "start.h"

int quit_flag = 0;
pthread_t threads[7];


void start_threads() {
  pthread_create(&(threads[0]), NULL, (void*)&thread_event_queue, (void*)&quit_flag);
  pthread_create(&(threads[1]), NULL, (void*)&thread_node_info_updater, (void*)&quit_flag);
  pthread_create(&(threads[2]), NULL, (void*)&thread_deposit_detector, (void*)&quit_flag);
  pthread_create(&(threads[3]), NULL, (void*)&thread_send_transaction, (void*)&quit_flag);
  pthread_create(&(threads[4]), NULL, (void*)&thread_spent_from_detector, (void*)&quit_flag);
  pthread_create(&(threads[5]), NULL, (void*)&thread_address_balance_detector, (void*)&quit_flag);
  pthread_create(&(threads[6]), NULL, (void*)&thread_sent_transaction_confirmer, (void*)&quit_flag);
}

void join_threads() {
  pthread_join(threads[0], NULL);
  pthread_join(threads[1], NULL);
  pthread_join(threads[2], NULL);
  pthread_join(threads[3], NULL);
  pthread_join(threads[4], NULL);
  pthread_join(threads[5], NULL);
  pthread_join(threads[6], NULL);
}
void shutdown_threads() {
  quit_flag = 1;
}
