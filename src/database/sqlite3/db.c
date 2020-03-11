//
// Created by matth on 2/16/2020.
//
#include <stdio.h>
#include <stdlib.h>
#include "tables.h"
#include "../../iota-simplewallet.h"
#include "db.h"
int init_db() {
  sqlite3 *connection = NULL;
  char* path = get_config("database");
  if(!path) {
    log_wallet_error( "Unable to read database path from config", "");
    exit(1);
  }
  int rc = sqlite3_open(path, &connection);
  free(path);
  if(rc) {
    log_wallet_error( "Can't open database: %s", sqlite3_errmsg(connection));
    sqlite3_close(connection);
    exit(1);
  }
  create_tables(connection);
  sqlite3_close(connection);
  return 0;
}

sqlite3* get_db_handle() {
  sqlite3* handle = NULL;
  char* path = get_config("database");
  if(!path) {
    log_wallet_error( "Unable to read database path from config", "");
    return NULL;
  }
  int rc = sqlite3_open(path, &handle);
  free(path);
  if(rc) {
    log_wallet_error( "Can't open database: %s", sqlite3_errmsg(handle));
    sqlite3_close(handle);
    return NULL;
  }
  sqlite3_exec(handle, "PRAGMA foreign_keys = ON", 0 ,0 ,0);

  return handle;
}

void close_db_handle(sqlite3* handle) {
    sqlite3_close(handle);
}