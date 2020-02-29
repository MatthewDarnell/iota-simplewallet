//
// Created by matth on 2/29/2020.
//

#ifndef IOTA_SIMPLEWALLET_IOTA_SIMPLEWALLET_H
#define IOTA_SIMPLEWALLET_IOTA_SIMPLEWALLET_H
#include <stdint.h>
#include <sqlite3.h>

//Initialization/Shutdown Functions
void init_logger();
int init_db();
int init_crypto();
void init_iota();
sqlite3* get_db_handle();


void close_db_handle(sqlite3* handle);
void shutdown_iota();
void shutdown_config();


/*
 *
 *   Start/Stop the IOTA Threads
 *
*/

void start_threads();
void join_threads();
void shutdown_threads();

/*
 *
 *   Logging functions
 *
*/
enum LOG_LEVEL {
    INFO = 1,
    DEBUG = 2,
    E = 3,
    FATAL = 4,
};
void _log(enum LOG_LEVEL, const char* format, ...);
#define log_wallet_info(x, ...) _log(INFO, x,  __VA_ARGS__);
#define log_wallet_debug(x, ...) _log(DEBUG, x,  __VA_ARGS__);
#define log_wallet_error(x, ...) _log(E, x,  __VA_ARGS__);
#define log_wallet_fatal(x, ...) _log(FATAL, x,  __VA_ARGS__);



/*
 *
 *   Configuration functions
 *
*/

//Load  initial config  from  file path
//@path NULL for defaults, file path for  json  file
//returns 0 for success, -1 for  failure
int load_config(const char *path);

//Shutdown the config file
void shutdown_config();

//Get Current  Configuration Value
//returns NULL if not found, otherwise  remember to free!
char* get_config(const char *key);

//Set  Configuration Value
//@save 1: save to file, otherwise only set  in-memory
int set_config(const char  *key, const char *value, int8_t save);



/*
 *
 *   Account functions
 *
*/

int __create_account(const char* username, char* password, const char* imported_seed);
#define create_account(username, password) __create_account(username, password, NULL)
#define import_account(username, password, seed) __create_account(username, password, seed)
char* get_accounts();
int verify_login(const char* username, char* password);



/*
 *
 *   Address functions
 *
*/

char* get_new_address(char* username);


/*
 *
 *   Transaction functions
 *
*/
char* get_incoming_transaction_by_hash(char* hash);
char* get_incoming_transactions(char* username, int offset, int num);



#endif //IOTA_SIMPLEWALLET_IOTA_SIMPLEWALLET_H
