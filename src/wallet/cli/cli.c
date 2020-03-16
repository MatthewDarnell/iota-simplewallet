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
                    "\t\tdelete_account <username> <password>\n"
                    "\t\timport_account <username> <password> <seed>\n"
                    "\t\texport_account_state <username> <password> <path> --- Write a synced account state to a file\n"
                    "\t\timport_account_state <password> <path> --- Import a file and create a new synced account\n"
                    "\t\tlogin <username> <password>\n"
                    "\t\tdecrypt_seed <username> <password>\n"
                    "\t\tget_all_accounts\n"
                    "\tAddresses:\n"
                    "\t\tget_new_address <username>\n"
                    "\tTransactions:\n"
                    "\t\tsend_transaction <username> <password> <destination_address> <amount>\n"
                    "\t\tget_all_transactions <username> <offset> <limit>\n"
                    "\t\tget_transaction <hash>\n"
                    "\t\tget_all_sent_transactions <offset> <limit>\n"
                    "\t\tget_sent_transaction <hash>\n"
                    "\tEvents:\n"
                    "\t\tget_all_events\n"
                    "\tMisc:\n"
                    "\t\thelp\n"
                    "\t\tget_node_status\n"
                    "\t\tset_node <host> <port>\n"
                    "\t\tquit\n"
                    "\n----------\n\n";
    log_wallet_info("%s", h);
    printf("%s", h);
  } else if(strcasecmp(command, "create_account") == 0) {
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
  } else if(strcasecmp(command, "delete_account") == 0) {
    username = strtok_r(NULL, " ", &saveptr);
    password = strtok_r(NULL, " ", &saveptr);
    if(!username || !password) {
      fprintf(stderr, "Invalid usage: delete_account <username> <password>\n");
      log_wallet_error("Invalid usage: delete_account <username> <password>\n", "");
      return -1;
    }
    if(0 == delete_account(username, password)) {
      printf("OK\n");
      log_wallet_info("Deleted User %s\n", username);
    } else {
      printf("ERROR\n");
      log_wallet_error("Error Deleting User\n", "");
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
    username = strtok_r(NULL, " ", &saveptr);
    password = strtok_r(NULL, " ", &saveptr);
    if(!username || !password) {
      fprintf(stderr, "Invalid usage:  login <username> <password>\n");
      log_wallet_error("Invalid usage:  login <username> <password>\n", "");
      return -1;
    }
    if(0 == verify_login(username, password, 1)) {
      printf("OK\n");
      log_wallet_info("Successfully Logged In User\n", "");
    } else {
      printf("ERROR\n");
      log_wallet_error("Error logging in\n", "");
    }
  } else if(strcasecmp(command, "decrypt_seed") == 0) {
    username = strtok_r(NULL, " ", &saveptr);
    password = strtok_r(NULL, " ", &saveptr);
    if(!username || !password) {
      fprintf(stderr, "Invalid usage: decrypt_seed <username> <password>\n");
      log_wallet_error("Invalid usage: decrypt_seed <username> <password>\n", "");
      return -1;
    }
    char seed[128] = { 0 };
    if(0 == decrypt_seed(seed, 128, username, password)) {
      printf("%s\n", seed);
      log_wallet_info("Successfully Decrypted Seed\n", "");
      memset(seed, 0, 128);
    } else {
      printf("ERROR\n");
      log_wallet_error("Error logging in\n", "");
    }
  } else if(strcasecmp(command, "export_account_state") == 0) {
    username = strtok_r(NULL, " ", &saveptr);
    password = strtok_r(NULL, " ", &saveptr);
    char* path = strtok_r(NULL, " ", &saveptr);
    if(!username || !password || !path) {
      fprintf(stderr, "Usage: export_account_state <username> <password> <path>\n");
      log_wallet_error("Usage: export_account_state <username> <password> <path>\n", "");
    } else {
      int state_written = export_account_state(username, password, path);
      printf("%s\n", state_written == 0 ? "OK" : "NOT OK");
      log_wallet_info("%s\n", state_written == 0 ? "OK" : "NOT OK");
    }
  } else if(strcasecmp(command, "import_account_state") == 0) {
    password = strtok_r(NULL, " ", &saveptr);
    char* path = strtok_r(NULL, " ", &saveptr);
    if( !password || !path) {
      fprintf(stderr, "Usage: import_account_state <password> <path>\n");
      log_wallet_error("Usage: import_account_state <password> <path>\n", "");
    } else {
      int state_imported = import_account_state(password, path);
      printf("%s\n", state_imported == 0 ? "OK" : "NOT OK");
      log_wallet_info("%s\n", state_imported == 0 ? "OK" : "NOT OK");
    }
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
    username = strtok_r(NULL, " ", &saveptr);
    if(!username) {
      fprintf(stderr, "Usage: get_new_address <username>\n");
      log_wallet_error("Usage: get_new_address <username>\n", "");
    } else {
      char* address = NULL;
      address = get_new_address(username);
      if(!address) {
        log_wallet_error("Unable to get a new address", "");
      } else {
        printf("%s\n", address);
        log_wallet_info("%s\n", address);
        free(address);
      }
    }
  } else if(strcasecmp(command, "get_all_transactions") == 0) {

    username = strtok_r(NULL, " ", &saveptr);
    char* str_offset = strtok_r(NULL, " ", &saveptr);
    char* str_limit = strtok_r(NULL, " ", &saveptr);

    if(!username || !str_limit || !str_offset) {
      fprintf(stderr, "Usage: get_all_transactions <username> <offset> <limit>\n");
      log_wallet_error("Usage: get_all_transactions <username> <offset> <limit>\n", "");
    } else {
      int offset = atoi(str_offset);
      int limit = atoi(str_limit);
      char* txs = get_incoming_transactions(username, offset, limit);
      printf("%s\n", txs);
      log_wallet_info("%s\n", txs);
      free(txs);
    }
  } else if(strcasecmp(command, "get_all_sent_transactions") == 0) {

    username = strtok_r(NULL, " ", &saveptr);
    char* str_offset = strtok_r(NULL, " ", &saveptr);
    char* str_limit = strtok_r(NULL, " ", &saveptr);

    if(!username || !str_limit || !str_offset) {
      fprintf(stderr, "Usage: get_all_sent_transactions <username> <offset> <limit>\n");
      log_wallet_error("Usage: get_all_sent_transactions <username> <offset> <limit>\n", "");
    } else {
      int offset = atoi(str_offset);
      int limit = atoi(str_limit);
      char* txs = get_outgoing_transactions(username, offset, limit);
      printf("%s\n", txs);
      log_wallet_info("%s\n", txs);
      free(txs);
    }
  } else if(strcasecmp(command, "send_transaction") == 0) {
    username = strtok_r(NULL, " ", &saveptr);
    password = strtok_r(NULL, " ", &saveptr);
    if(!username || !password) {
      fprintf(stderr, "Invalid usage: send_transaction <username> <password> <destination_address> <amount>\n");
      log_wallet_error("Invalid usage: send_transaction <username> <password> <destination_address> <amount>\n", "");
      return -1;
    }
    char* destination = strtok_r(NULL, " ", &saveptr);
    uint64_t amount = strtoull(strtok_r(NULL, " ", &saveptr), NULL, 10);
    int res = create_transaction(username, password, destination, amount);
    printf("%s\n", res == 0 ? "OK" : "NOT OK");
    log_wallet_info("%s\n", res == 0 ? "OK" : "NOT OK");
  } else if(strcasecmp(command, "get_transaction") == 0) {
    char* hash = strtok_r(NULL, " ", &saveptr);
    char* tx = get_incoming_transaction_by_hash(hash);
    printf("%s\n", tx);
    log_wallet_info("%s\n", tx);
    free(tx);
  } else if(strcasecmp(command, "get_sent_transaction") == 0) {
    char* hash = strtok_r(NULL, " ", &saveptr);
    char* tx = get_outgoing_transaction_by_hash(hash);
    printf("%s\n", tx);
    log_wallet_info("%s\n", tx);
    free(tx);
  }


  //Events
  else if(strcasecmp(command, "get_all_events") == 0) {
    char* events = get_valid_events();
    printf("%s\n", events);
    log_wallet_info("%s\n", events);
    free(events);
  } else if(strcasecmp(command, "set_node") == 0) {
    char* host = strtok_r(NULL, " ", &saveptr);
    char* str_port = strtok_r(NULL, " ", &saveptr);
    if(!host || !str_port) {
      fprintf(stderr, "Usage: set_node <host> <port>\t--- <host> should be in the form https://host.com\n");
      log_wallet_error("Usage: set_node <host> <port>\t--- <host> should be in the form https://host.com", "");
    } else {
      int port = strtol(str_port, NULL, 10);
      if(!port) {
        fprintf(stderr, "Invalid Port %s\n", str_port);
        log_wallet_error("Invalid Port %s\n", str_port);
      } else {
        int ret_val = set_node(host, port);
        printf("%s\n", ret_val == 0 ? "OK": "NOT OK");
        log_wallet_info("%s\n", ret_val == 0 ? "OK": "NOT OK");
      }
    }
  }

  else {
    fprintf(stderr, "Invalid Option. Try <help> to see all available options\n");
    log_wallet_error("Invalid Option. Try <help> to see all available options\n", "");
  }





  return 0;
}
