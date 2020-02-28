//
// Created by matth on 2/28/2020.
//

#ifndef IOTA_SIMPLEWALLET_CLI_H
#define IOTA_SIMPLEWALLET_CLI_H
#include <sqlite3.h>
int parse_cli(int argc, char *argv[]);
int parse_command(sqlite3* db, char* buf, int* quit_flag);
#endif //IOTA_SIMPLEWALLET_CLI_H
