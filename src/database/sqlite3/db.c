//
// Created by matth on 2/16/2020.
//
#include <stdio.h>
#include <stdlib.h>
#include <sqlite3.h>
#include "../../config/config.h"
#include "tables.h"
#include "db.h"
sqlite3 *connection = NULL;
int init_db() {
  char* path = get_config("database");
  if(!path) {
    fprintf(stderr, "Unable to read database path from config\n");
    exit(1);
  }
  int rc = sqlite3_open(path, &connection);
  free(path);
  if(rc) {
    fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(connection));
    sqlite3_close(connection);
    exit(1);
  }
  create_tables(connection);
  return 0;
}

void close_db() {
    sqlite3_close(connection);
}
