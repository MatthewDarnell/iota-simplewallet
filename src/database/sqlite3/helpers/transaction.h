//
// Created by matth on 2/28/2020.
//

#ifndef IOTA_SIMPLEWALLET_TRANSACTION_H
#define IOTA_SIMPLEWALLET_TRANSACTION_H
char* get_incoming_transaction_by_hash(char* hash);
char* get_incoming_transactions(char* username, int offset, int num);
#endif //IOTA_SIMPLEWALLET_TRANSACTION_H
