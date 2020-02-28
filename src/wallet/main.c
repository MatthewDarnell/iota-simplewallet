
#ifdef WIN32
#include <windows.h>
#define sleep(x) Sleep(x)
#endif

#include <stdio.h>
#include "../config/config.h"
#include "../database/sqlite3/db.h"
#include "../database/sqlite3/helpers/store_iota_inputs.h"
#include "../crypto/crypt.h"
#include "../iota/api.h"
#include "../database/sqlite3/helpers/account.h"

#include <pthread.h>
#include "../config/logger.h"
#include "../thread/deposit_detector.h"
#include "cli/cli.h"
int main(int argc, char *argv[]) {

 /*

  TODO: filter out known txs in deposit detector before calling get inclusion

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

  //Init Database and create tables if first run
  init_db();

  //Init Crypto
  init_crypto();

  //Init iota
  init_iota();

  sqlite3* db = get_db_handle();


  int quit_flag = 0;

  log_wallet_info("IOTA Wallet Started. Enter <quit> to shutdown.\n", "");
  printf("IOTA Wallet Started. Enter <quit> to shutdown.\n");

  pthread_t t;
  pthread_create(&t, NULL, (void*)&thread_deposit_detector, (void*)&quit_flag);
  char buf[256] = { 0 };
  while(quit_flag == 0) {
    if(quit_flag != 0) {
      break;
    }
    fgets(buf, 256, stdin);
    char *c = strtok(buf, "\n");
    printf("%s\n", c);
    parse_command(db, c, &quit_flag);
    sleep(1000);
  }
  pthread_join(t, NULL);


  close_db_handle(db);
  shutdown_iota();

  shutdown_config();

  return 0;
}
