
#ifdef WIN32
#include <windows.h>
#else
#include <unistd.h>
#define Sleep(x) sleep(x/1000)
#endif

#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include "../iota-simplewallet.h"
#include "cli/cli.h"


static void read_cli(void* args) {
  int* quit_flag = (int*)args;
  char buf[256] = { 0 };
  while(*quit_flag == 0) {
    if(*quit_flag != 0) {
      break;
    }
    fgets(buf, 256, stdin);
    char *c = strtok(buf, "\n");
    // printf("%s\n", c);
    parse_command(c, quit_flag);
    fflush (stdin);
    Sleep(100);
  }

  pthread_exit(0);
}


void* node_updated_callback(const char* value) {
  printf("%s fired!!! Got <%s>\n", __func__, value);
  return NULL;
}

void* balance_changed_callback(const char* value) {
  printf("%s fired!!! Got <%s>\n", __func__, value);
  return NULL;
}

void* transaction_received_callback(const char* value) {
  printf("%s fired!!! Got <%s>\n", __func__, value);
  return NULL;
}

void* transaction_received_confirmed_callback(const char* value) {
  printf("%s fired!!! Got <%s>\n", __func__, value);
  return NULL;
}

void* transaction_sent_callback(const char* value) {
  printf("%s fired!!! Got <%s>\n", __func__, value);
  return NULL;
}

void* sent_transaction_confirmed_callback(const char* value) {
  printf("%s fired!!! Got <%s>\n", __func__, value);
  return NULL;
}
int main(int argc, char *argv[]) {

 /*
  TODO: Add api call for exporting seed and/or account state to file

  */

  //Init Configuration
  if(argc > 0) {
    parse_cli(argc, argv);
  }

  init_iota_simplewallet(".");
  init_events();



  int quit_flag = 0;

  printf("\n\t\t--------------------------------------\n"
         "\t\t\tWelcome to IOTA Simple Wallet!\n"
         "\t\t--------------------------------------\n\n"
         "Enter <help> to see menu\nEnter <quit> to shutdown.\n");

  log_wallet_info("IOTA CLI Wallet Started", "");

  //register_callback("node_updated", &node_updated_callback);
  register_callback("balance_changed", &balance_changed_callback);
  register_callback("transaction_received", &transaction_received_callback);
  register_callback("transaction_received_confirmed", &transaction_received_confirmed_callback);
  register_callback("transaction_sent", &transaction_sent_callback);
  register_callback("sent_transaction_confirmed", &sent_transaction_confirmed_callback);

  start_threads();

  pthread_t t;
  pthread_create(&(t), NULL, (void*)&read_cli, (void*)&quit_flag);


  pthread_join(t, NULL);
  join_threads();

  shutdown_iota_simplewallet();
  shutdown_events();

  return 0;
}
