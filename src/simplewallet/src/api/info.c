//
// Created by matth on 3/10/2020.
//

#include <stdlib.h>
#include <sodium.h>
#include <cjson/cJSON.h>
#include "../iota/api.h"
#include "../iota-simplewallet.h"

char* get_node_status() {
  cJSON* json = NULL;
  char* info = get_config("info");
  if(!info) {
    return NULL;
  }
  json = cJSON_Parse(info);
  free(info);

  cJSON* node = get_node();
  cJSON_AddItemToObject(json, "node", node);
  char* ret_val = cJSON_Print(json);
  cJSON_Delete(json);
  return ret_val;
}

char* parse_debug_command(const char* cmd) {
  char* ret_val = NULL;

  char* username, *password, *saveptr;
  if(!cmd) {
    return ret_val;
  }

  char* command = strdup(cmd);
  if(!command) {
    return ret_val;
  }

  char* c = strtok_r(command, " ", &saveptr);

  if(!c) {
    return ret_val;
  }

  if(strcasecmp(c, "help") == 0) {
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
    ret_val = strdup(h);
  }

    //Accounts
  else if(strcasecmp(c, "create_account") == 0) {
    username = strtok_r(NULL, " ", &saveptr);
    password = strtok_r(NULL, " ", &saveptr);
    if(!username || !password) {
      ret_val = strdup("Invalid usage:  create_account <username> <password>");
    } else {
      if (0 == create_account(username, password)) {
        ret_val = strdup("OK");
        log_wallet_info("Created User %s\n", username);
      } else {
        ret_val = strdup("ERROR");
        log_wallet_error("Error Creating User\n", "");
      }
    }
  }
  else if(strcasecmp(c, "delete_account") == 0) {
    username = strtok_r(NULL, " ", &saveptr);
    password = strtok_r(NULL, " ", &saveptr);
    if(!username || !password) {
      ret_val = strdup("Invalid usage: delete_account <username> <password>");
    } else {
      if (0 == delete_account(username, password)) {
        ret_val = strdup("OK");
        log_wallet_info("Deleted User %s\n", username);
      } else {
        ret_val = strdup("ERROR");
        log_wallet_error("Error Deleting User\n", "");
      }
    }
  }
  else if(strcasecmp(c, "import_account") == 0) {
    username = strtok_r(NULL, " ", &saveptr);
    password = strtok_r(NULL, " ", &saveptr);
    char* seed = strtok_r(NULL, " ", &saveptr);
    if(!username || !password || !seed) {
      ret_val = strdup("Invalid usage: import_account <username> <password> <seed>");
    } else {
      if (0 == import_account(username, password, seed)) {
        ret_val = strdup("OK");
        log_wallet_info("Created User %s\n", username);
      } else {
        ret_val = strdup("ERROR");
        log_wallet_error("Error Creating User\n", "");
      }
    }
  }
  else if(strcasecmp(c, "login") == 0) {
    username = strtok_r(NULL, " ", &saveptr);
    password = strtok_r(NULL, " ", &saveptr);
    if(!username || !password) {
      ret_val = strdup("Invalid usage: login <username> <password>");
    } else {
      if (0 == verify_login(username, password, 1)) {
        ret_val = strdup("OK");
        log_wallet_info("Successfully Logged In User\n", "");
      } else {
        ret_val = strdup("ERROR");
        log_wallet_error("Error logging in\n", "");
      }
    }
  }
  else if(strcasecmp(c, "decrypt_seed") == 0) {
    username = strtok_r(NULL, " ", &saveptr);
    password = strtok_r(NULL, " ", &saveptr);
    if(!username || !password) {
      ret_val = strdup("Invalid usage: decrypt_seed <username> <password>");
    } else {
      char seed[128] = {0};
      if (0 == decrypt_seed(seed, 128, username, password)) {
        ret_val = strdup(seed);
        sodium_memzero(seed, 128);
      } else {
        ret_val = strdup("ERROR");
      }
    }
  }
  else if(strcasecmp(c, "export_account_state") == 0) {
    username = strtok_r(NULL, " ", &saveptr);
    password = strtok_r(NULL, " ", &saveptr);
    char* path = strtok_r(NULL, " ", &saveptr);
    if(!username || !password || !path) {
      ret_val = strdup("Usage: export_account_state <username> <password> <path>");
    } else {
      int state_written = export_account_state(username, password, path);
      ret_val = strdup(state_written == 0 ? "OK" : "NOT OK");
    }
  }
  else if(strcasecmp(c, "import_account_state") == 0) {
    password = strtok_r(NULL, " ", &saveptr);
    char* path = strtok_r(NULL, " ", &saveptr);
    if( !password || !path) {
      ret_val = strdup("Usage: import_account_state <password> <path>");
    } else {
      int state_imported = import_account_state(password, path);
      ret_val = strdup(state_imported == 0 ? "OK" : "NOT OK");
    }
  }
  else if(strcasecmp(c, "get_all_accounts") == 0) {
    char* accounts = get_accounts();
    ret_val = strdup(accounts);
  }

    //Addresses
  else if(strcasecmp(c, "get_new_address") == 0) {
    username = strtok_r(NULL, " ", &saveptr);
    if(!username) {
      ret_val = strdup("Usage: get_new_address <username>");
    } else {
      ret_val = get_new_address(username);
    }
  }

    //Transactions
  else if(strcasecmp(c, "get_all_transactions") == 0) {
    username = strtok_r(NULL, " ", &saveptr);
    char* str_offset = strtok_r(NULL, " ", &saveptr);
    char* str_limit = strtok_r(NULL, " ", &saveptr);

    if(!username || !str_limit || !str_offset) {
      ret_val = strdup("Usage: get_all_transactions <username> <offset> <limit>");
    } else {
      uint32_t offset = strtol(str_offset, NULL, 10);
      uint32_t limit = strtol(str_limit, NULL, 10);
      ret_val = get_incoming_transactions(username, offset, limit);
    }
  }
  else if(strcasecmp(c, "get_all_sent_transactions") == 0) {
    username = strtok_r(NULL, " ", &saveptr);
    char* str_offset = strtok_r(NULL, " ", &saveptr);
    char* str_limit = strtok_r(NULL, " ", &saveptr);

    if(!username || !str_limit || !str_offset) {
      ret_val = strdup("Usage: get_all_sent_transactions <username> <offset> <limit>");
    } else {
      uint32_t offset = strtol(str_offset, NULL, 10);
      uint32_t limit = strtol(str_limit, NULL, 10);
      ret_val = get_outgoing_transactions(username, offset, limit);
    }
  }
  else if(strcasecmp(c, "send_transaction") == 0) {
    username = strtok_r(NULL, " ", &saveptr);
    password = strtok_r(NULL, " ", &saveptr);
    char* destination = strtok_r(NULL, " ", &saveptr);
    char* str_amount = strtok_r(NULL, " ", &saveptr);
    if(!username || !password || !destination || !str_amount) {
      ret_val = strdup("Invalid usage: send_transaction <username> <password> <destination_address> <amount>");
    } else {
      uint64_t amount = strtoull(str_amount, NULL, 10);
      if (amount < 1) {
        ret_val = strdup("Invalid usage. Amount must be greater than 0");
      } else {
        int res = create_transaction(username, password, destination, amount);
        ret_val = strdup(res == 0 ? "OK" : "NOT OK");
      }
    }
  }
  else if(strcasecmp(c, "get_transaction") == 0) {
    char* hash = strtok_r(NULL, " ", &saveptr);
    if(!hash) {
      ret_val = strdup("Invalid usage: get_transaction <hash>");
    } else {
      ret_val = get_incoming_transaction_by_hash(hash);
    }
  }
  else if(strcasecmp(c, "get_sent_transaction") == 0) {
    char* hash = strtok_r(NULL, " ", &saveptr);
    if(!hash) {
      ret_val = strdup("Invalid usage: get_sent_transaction <hash>");
    } else {
      ret_val = get_outgoing_transaction_by_hash(hash);
    }
  }

    //Misc
  else if(strcasecmp(c, "get_node_status") == 0) {
    ret_val = get_node_status();
  }
  else if(strcasecmp(c, "set_node") == 0) {
    char* host = strtok_r(NULL, " ", &saveptr);
    char* str_port = strtok_r(NULL, " ", &saveptr);
    if(!host || !str_port) {
      ret_val = strdup("Usage: set_node <host> <port>\t--- <host> should be in the form host.com");
    } else {
      uint32_t port = strtol(str_port, NULL, 10);
      if(!port) {
        ret_val = strdup("Invalid Port");
      } else {
        int res = set_node(host, port);
        ret_val = strdup(res == 0 ? "OK": "NOT OK");
      }
    }
  }

    //Events
  else if(strcasecmp(c, "get_all_events") == 0) {
    ret_val = get_valid_events();
  }
  else {
    ret_val = strdup("Invalid Option. Try <help> to see all available options");
  }

  free(command);
  return ret_val;
}
