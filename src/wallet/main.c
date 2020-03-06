
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
#include "../thread/deposit_detector.h"
#include "cli/cli.h"


static void read_cli(void* args) {
  int* quit_flag = (int*)args;
  char buf[256] = { 0 };
  printf("Welcome to IOTA wallet CLI. Enter <help> to see available commands\n");
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


int main(int argc, char *argv[]) {

 /*

  TODO: filter out known txs in deposit detector before calling get inclusion

  TODO: dont expose db handles to user api

      TODO: add callback mechanism for events to UI

      TODO: api
          GetNewAddress(username, password) --->
              1. Verify Login (this will  auto-gen more fresh addresses)
              2. Get Next Clean Address
              3. Return Address
          GetAddresses(username) --->
            1. Get all Addresses for this username
            2. Return Addresses
          GetTransactions(address) -->
              1. Fetch Incoming Transactions to and Outgoing Transactions from address
              2. Return Transactions

      TODO threads:
        incoming transaction detector X
        outgoing transaction sender
        sent transaction promoter


  */

  //Init Configuration
  load_config(NULL);
  if(argc > 0) {
    parse_cli(argc, argv);
  }

  init_db();
  init_crypto();
  init_iota();



  int quit_flag = 0;

  log_wallet_info("IOTA Wallet Started. Enter <quit> to shutdown.\n", "");
  printf("IOTA Wallet Started. Enter <quit> to shutdown.\n");


  start_threads();

  pthread_t t;
  pthread_create(&(t), NULL, (void*)&read_cli, (void*)&quit_flag);


  pthread_join(t, NULL);
  join_threads();

  shutdown_iota();

  shutdown_config();

  return 0;
}
