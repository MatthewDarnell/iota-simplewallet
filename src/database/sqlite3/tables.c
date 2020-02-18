//
// Created by matth on 2/18/2020.
//

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <sqlite3.h>

//db should already be opened
int create_tables(sqlite3* db) {
  char *zErrMsg = 0;
  int rc, i;

  const int num_tables = 4;
  const char* tables[] = {
    "CREATE TABLE IF NOT EXISTS account(serial integer primary key autoincrement, seed_ciphertext text, salt text, nonce text, username text, balance integer, is_synced integer default 0, created_at timestamp default current_timestamp)",
    "CREATE TABLE IF NOT EXISTS address(serial integer primary key autoincrement, address text, offset integer, account integer references account(serial), is_change integer default 0, spent_from integer default 0, created_at timestamp default current_timestamp)",
    "CREATE TABLE IF NOT EXISTS incoming_transaction(serial integer primary key autoincrement, time timestamp, amount integer, address text references address(address), confirmed integer default 0, bundle text, hash text, created_at timestamp default current_timestamp)",
    "CREATE TABLE IF NOT EXISTS outgoing_transaction(serial integer primary key autoincrement, amount integer, input_address text references address(address), dest_address text, change_address text references address(address), trytes text, sent integer default 0, created_at timestamp default current_timestamp)"
  };

  for(i = 0; i < num_tables; i++) {
    const char* query = tables[i];
    rc = sqlite3_exec(db, query, NULL, 0, &zErrMsg);
    if( rc != SQLITE_OK ){
      fprintf(stderr, "SQL error on query %d: %s\nBailing out!\n", i, zErrMsg);
      sqlite3_free(zErrMsg);
      exit(1);
    }
    printf("Created all Database Tables successfully\n");
  }
  return 0;
}