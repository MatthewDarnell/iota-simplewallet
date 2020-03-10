//
// Created by matth on 2/28/2020.
//

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../../iota-simplewallet.h"
#include "cli.h"

const int num_cli_commands = 5;
const char* cli_commands[] = {
  "", //set to --flag, will set_config --flag <value>
  "",
  "",
  "",
  "",
};


int parse_cli(int argc, char *argv[]) {
  int i, j;
  for(i=0; i < argc; i++) {
    char* arg = argv[i];

    for(j = 0; j < num_cli_commands; j++) {
      const char* command = cli_commands[j];
      if(strcasecmp(arg, command) == 0) {
        if(i >= argc -1) {
          fprintf(stderr, "Invalid cli options passed: %s\n", command);
          break;
        }
        const char* value = argv[i+1];
        set_config(command+2, value, 0);
        i++;
        break;
      }
    }
  }
  return 0;
}



const int num_commands = 3;
const char* commands[] = {
  "create_account",
  "login",
  "quit",
  "",
  "",
};


int parse_command(char* buf, int* quit_flag) {
  char* username, *password, *saveptr;
  if(!buf) {
    return 0;
  }
  char* command = strtok_r(buf, " ", &saveptr);
  if(strcasecmp(command, "quit") == 0) {
    log_wallet_info("Got quit command, shutting down.", "");
    shutdown_threads();
    pthread_exit(NULL);
  } else if(strcasecmp(command, "help") == 0) {
    const char *h = "\n----------\n"
                    "IOTA Wallet CLI Usage: \n"
                    "\tAccounts:\n"
                    "\t\tcreate_account <username> <password>\n"
                    "\t\timport_account <username> <password> <seed>\n"
                    "\t\tlogin <password>\n"
                    "\t\tswitch_account <username> -- sets <username> as the default account. Pass null to other functions to use the default account.\n"
                    "\t\tget_main_account -- returns main account that is currently set.\n"
                    "\t\tget_all_accounts\n"
                    "\tAddresses:\n"
                    "\t\tget_new_address\n"
                    "\tTransactions:\n"
                    "\t\tsend_transaction <password> <destination_address> <amount>\n"
                    "\t\tget_all_transactions <offset> <limit>\n"
                    "\t\tget_transaction <hash>\n"
                    "\tMisc:\n"
                    "\t\thelp\n"
                    "\t\tget_node_status\n"
                    "\t\tquit\n"
                    "\n----------\n\n";
    log_wallet_info("%s", h);
    printf("%s", h);
  } else if(strcasecmp(command, "get_all_accounts") == 0) {
    char* accounts = get_accounts();
    printf("%s\n", accounts);
    log_wallet_info("%s\n", accounts);
    free(accounts);
  } else if(strcasecmp(command, "get_node_status") == 0) {
    char* status = get_node_status();
    printf("%s\n", status);
    log_wallet_info("%s\n", status);
    free(status);
  } else if(strcasecmp(command, "get_new_address") == 0) {
    char* address = NULL;
    address = get_new_address(NULL);
    if(!address) {
      log_wallet_error("Unable to get a new address", "");
    } else {
      printf("%s\n", address);
      log_wallet_info("%s\n", address);
      free(address);
    }
  } else if(strcasecmp(command, "get_all_transactions") == 0) {

    char* str_offset = strtok_r(NULL, " ", &saveptr);
    char* str_limit = strtok_r(NULL, " ", &saveptr);

    if(!str_limit || !str_offset) {
      fprintf(stderr, "Usage: get_all_transactions <offset> <limit>\n");
      log_wallet_error("Usage: get_all_transactions <offset> <limit>\n", "");
    } else {
      int offset = atoi(str_offset);
      int limit = atoi(str_limit);
      char* txs = get_incoming_transactions(NULL, offset, limit);
      printf("%s\n", txs);
      log_wallet_info("%s\n", txs);
      free(txs);
    }
  } else if(strcasecmp(command, "send_transaction") == 0) {
    password = strtok_r(NULL, " ", &saveptr);
    if(!password) {
      fprintf(stderr, "Invalid usage: send_transaction <password>\n");
      log_wallet_error("Invalid usage: send_transaction <password>\n", "");
      return -1;
    }
    char* destination = strtok_r(NULL, " ", &saveptr);
    uint64_t amount = strtoull(strtok_r(NULL, " ", &saveptr), NULL, 10);
    int res = create_transaction(NULL, password, destination, amount);
    printf("%s\n", res == 0 ? "OK" : "NOT OK");
    log_wallet_info("%s\n", res == 0 ? "OK" : "NOT OK");
  } else if(strcasecmp(command, "get_transaction") == 0) {
    char* hash = strtok_r(NULL, " ", &saveptr);
    char* tx = get_incoming_transaction_by_hash(hash);
    printf("%s\n", tx);
    log_wallet_info("%s\n", tx);
    free(tx);
  }
  else if(strcasecmp(command, "create_account") == 0) {
    username = strtok_r(NULL, " ", &saveptr);
    password = strtok_r(NULL, " ", &saveptr);
    if(!username || !password) {
      fprintf(stderr, "Invalid usage:  create_account <username> <password>\n");
      log_wallet_error("Invalid usage:  create_account <username> <password>\n", "");
      return -1;
    }
    if(0 == create_account(username, password)) {
      printf("OK\n");
      log_wallet_info("Created User %s\n", username);
    } else {
      printf("ERROR\n");
      log_wallet_error("Error Creating User\n", "");
    }
  } else if(strcasecmp(command, "import_account") == 0) {
    username = strtok_r(NULL, " ", &saveptr);
    password = strtok_r(NULL, " ", &saveptr);
    char* seed = strtok_r(NULL, " ", &saveptr);
    if(!username || !password || !seed) {
      fprintf(stderr, "Invalid usage:  import_account <username> <password> <seed>\n");
      log_wallet_error("Invalid usage:  import_account <username> <password> <seed>\n", "");
      return -1;
    }
    if(0 == import_account(username, password, seed)) {
      printf("OK\n");
      log_wallet_info("Created User %s\n", username);
    } else {
      printf("ERROR\n");
      log_wallet_error("Error Creating User\n", "");
    }
  } else if(strcasecmp(command, "login") == 0) {
    password = strtok_r(NULL, " ", &saveptr);
    if(!password) {
      fprintf(stderr, "Invalid usage:  login <password>\n");
      log_wallet_error("Invalid usage:  login <password>\n", "");
      return -1;
    }
    if(0 == verify_login(NULL, password, 1)) {
      printf("OK\n");
      log_wallet_info("Successfully Logged In User\n", "");
    } else {
      printf("ERROR\n");
      log_wallet_error("Error logging in\n", "");
    }
  } else if(strcasecmp(command, "switch_account") == 0) {
    username = strtok_r(NULL, " ", &saveptr);
    if(!username) {
      fprintf(stderr, "Invalid usage: switch_account <username>\n");
      log_wallet_error("Invalid usage: switch_account <username>\n", "");
      return -1;
    }
    if(0 == switch_account(username)) {
      printf("OK\n");
      log_wallet_info("Successfully Switched Account to User: %s\n", username);
    } else {
      printf("ERROR\n");
      log_wallet_error("Error Switching Account in\n", "");
    }
  }  else if(strcasecmp(command, "get_main_account") == 0) {
    char* acc = get_main_account();
    printf("%s\n", acc);
    log_wallet_info("%s\n", acc);
    free(acc);
  }

  else {
    fprintf(stderr, "Invalid Option. Try <help> to see all available options\n");
    log_wallet_error("Invalid Option. Try <help> to see all available options\n", "");
  }





  return 0;
}
