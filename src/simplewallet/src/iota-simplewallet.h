//
// Created by matth on 2/29/2020.
//

#ifdef __cplusplus
extern "C" {
#endif

#ifndef IOTA_SIMPLEWALLET_IOTA_SIMPLEWALLET_H
#define IOTA_SIMPLEWALLET_IOTA_SIMPLEWALLET_H
#include <stdint.h>

/*
 *
 *   Initialization/Shutdown Functions
 *
*/
int init_iota_simplewallet(const char* data_dir);
int shutdown_iota_simplewallet();

/*
 *
 *   Start/Stop the IOTA Wallet Threads
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
 *   Info functions
 *
*/

//Get current node info
char* get_node_status();

//Sets the node to use
int set_node(char* host, int port);


/*
 *
 *   Configuration functions
 *
*/

//Shutdown the config file
void shutdown_config();

//Get Current  Configuration Value
//returns NULL if not found, otherwise remember to free
char* get_config(const char *key);

//Set  Configuration Value
//@save 1: save to file, otherwise only set  in-memory
int set_config(const char* key, const char* value, int8_t save);



/*
 *
 *   Account functions
 *
*/

//Create an account, should use one of the following #define functions
int __create_account(const char* username, char* password, const char* imported_seed);

//Create a New IOTA account
//@username a Unique name
//@password a password
#define create_account(username, password) __create_account(username, password, NULL)

//Import an existing IOTA seed into a simplewallet account
//@seed an 81 character IOTA seed
#define import_account(username, password, seed) __create_account(username, password, seed)

//Delete an account
//This deletes all account data, addresses, and transactions
int delete_account(const char* username, char* password);

//Get all stored accounts
char* get_accounts();

//Verify that the username and password are valid
//@zero_password: if > 0, This will mem-zero the decrypted password and seed after verifying the password is correct
//@generate_inputs: If > 0, the users' account will be checked for having a sufficient pool of unused deposit addresses. Should use #define macro
int _verify_login(const char* username, char* password, int zero_password, int generate_inputs);
#define verify_login(username, password, zero_password) _verify_login(username, password, zero_password, 1)

//Decrypt a users' seed
//@out: the char[] in which to place the decrypted seed
//@out_max_le: the max length of @out
//@password: the users' password
int decrypt_seed(char* out, int out_max_len, const char* username, char* password);

//Export a synced users' account state
//@path: the output file to write
int export_account_state(const char* username, char* password, const char* path);

//Import a synced account from file
//@path: the output file to write
//Account must not exist already
int import_account_state(char* password, const char* path);

//Write a key/value pair to the db associated with this user
//return: Returns 0 if ok, -1 otherwise
int write_user_data(const char* username, const char* key, const char* value);

//return: Returns 0 if ok, -1 otherwise
int delete_user_data(const char* username, const char* key);

//return: Returns NULL if user or key does not exist, a json string otherwise containing {value: "value"}
char* read_user_data(const char* username, const char* key);
/*
 *
 *   Address functions
 *
*/

//Get a new Deposit Address
char* get_new_address(const char* username);

//Generate a user-defined number of Deposit Addresses and Save to Disk
int generate_num_addresses(const char* username, char* password, int num_addrs);


/*
 *
 *   Transaction functions
 *
*/

//Get a received transaction
//@hash: transaction hash
char* get_incoming_transaction_by_hash(const char* hash);

//Get received transactions
//@offset: the 0-based offset at which to start looking
//@num: the max count of transactions to return
char* get_incoming_transactions(const char* username, uint32_t offset, uint32_t num);

//Get a sent transaction
//@hash: transaction hash
char* get_outgoing_transaction_by_hash(const char* hash);

//Get sent transactions
//@offset: the 0-based offset at which to start looking
//@num: the max count of transactions to return
char* get_outgoing_transactions(const char* username, uint32_t offset, uint32_t num);


//Creates a new transaction to send
//@password: the password of the account
//@dest_address: the receiving address
//@value: the amount to send
//Returns 0 on success, < 1 if failure
int create_transaction(const char* username, char* password, const char* dest_address, uint64_t value);


/*
 *
 *   Event functions
 *
*/
int init_events();
void shutdown_events();
char* get_valid_events();
int register_callback(const char* event, void* (*cb)(const char*));


/*
 *
 *   Other functions
 *
*/

//Access this API at runtime
//@command: the full string, i.e. "get_new_address <username>"
// <help> will return all available options
char* parse_debug_command(const char* cmd);

#endif //IOTA_SIMPLEWALLET_IOTA_SIMPLEWALLET_H


#ifdef __cplusplus
}
#endif