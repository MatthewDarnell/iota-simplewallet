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

  const int num_tables = 5;
  const char* tables[] = {
    "CREATE TABLE IF NOT EXISTS account(serial integer primary key autoincrement, seed_ciphertext text not null, salt text not null, nonce text not null, username text unique not null, balance integer default 0, is_synced integer default 0, created_at timestamp default current_timestamp)",
    "CREATE TABLE IF NOT EXISTS address(serial integer primary key autoincrement, address text unique not null, offset integer not null, account text not null, is_change integer default 0, created_at timestamp default current_timestamp, unique(offset, account), foreign key (account) references account(username))",
    "CREATE TABLE IF NOT EXISTS incoming_transaction(serial integer primary key autoincrement, time timestamp, amount integer not null, address text references address(address), confirmed integer default 0, bundle text not null, hash text unique not null, created_at timestamp default current_timestamp)",
    "CREATE TABLE IF NOT EXISTS outgoing_transaction(serial integer primary key autoincrement, amount integer, dest_address text not null, change_address text references address(address), trytes text not null, sent integer default 0, bundle text, hash text unique, created_at timestamp default current_timestamp)",
    "CREATE TABLE IF NOT EXISTS input_to_output(serial integer primary key autoincrement, input text references incoming_transaction(hash), output text references outgoing_transaction(serial), unique(input, output))"
  };

  for(i = 0; i < num_tables; i++) {
    const char* query = tables[i];
    rc = sqlite3_exec(db, query, NULL, 0, &zErrMsg);
    if( rc != SQLITE_OK ){
      fprintf(stderr, "SQL error on query %d: %s\nBailing out!\n", i, zErrMsg);
      sqlite3_free(zErrMsg);
      exit(1);
    }
  }
  printf("Created all Database Tables successfully\n");
  return 0;
}