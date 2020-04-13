//
// Created by matth on 2/18/2020.
//

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <sqlite3.h>
#include "../../iota-simplewallet.h"
//db should already be opened
int create_tables(sqlite3* db) {
  char *zErrMsg = 0;
  int rc, i;

  const int num_tables = 5;
  const char* tables[] = {
    "CREATE TABLE IF NOT EXISTS account(serial integer primary key autoincrement, seed_ciphertext text not null, salt text not null, nonce text not null, username text unique not null, balance integer default 0, is_synced integer default 0, created_at integer(4) default (strftime('%s','now')))",
    "CREATE TABLE IF NOT EXISTS address(serial integer primary key autoincrement, address text unique not null, balance text not null default \"0\", offset integer not null, account text not null, is_change integer default 0, spent_from integer not null default 0, used integer default 0, created_at integer(4) default (strftime('%s','now')), unique(offset, account), foreign key (account) references account(username))",
    "CREATE TABLE IF NOT EXISTS incoming_transaction(serial integer primary key autoincrement, time timestamp not null, amount integer not null, address text references address(address), confirmed integer default 0, bundle text not null, hash text unique not null, created_at integer(4) default (strftime('%s','now')), unique(bundle, address, amount))",
    "CREATE TABLE IF NOT EXISTS outgoing_transaction(serial integer primary key autoincrement, amount integer, dest_address text not null, change_address text references address(address), confirmed integer default 0, trytes text not null, sent integer default 0, bundle text, hash text unique, created_at integer(4) default (strftime('%s','now')))",
    "CREATE TABLE IF NOT EXISTS user_data(username text references account(username), key text not null, value text not null, created_at integer(4) default (strftime('%s','now')), unique(username, key))"
  };

  const int num_misc = 1;
  const char* misc[] = {
    "CREATE TRIGGER IF NOT EXISTS balance_updater"
    " AFTER UPDATE ON address"
    " BEGIN"
    "  UPDATE account"
    "   SET balance=("
    "    SELECT SUM("
    "     CAST(balance as INTEGER))"
    "      FROM address"
    "      WHERE username=account"
    "   );"
    "END;"
  };

  for(i = 0; i < num_tables; i++) {
    const char* query = tables[i];
    rc = sqlite3_exec(db, query, NULL, 0, &zErrMsg);
    if( rc != SQLITE_OK ){
      log_wallet_error("SQL error on query %d: %sBailing out!", i, zErrMsg);
      sqlite3_free(zErrMsg);
      exit(1);
    }
  }

  for(i = 0; i < num_misc; i++) {
    const char* query = misc[i];
    rc = sqlite3_exec(db, query, NULL, 0, &zErrMsg);
    if( rc != SQLITE_OK ){
      log_wallet_error("SQL error on query %d: %s - (%s)\n Bailing out!", i, zErrMsg, query);
      sqlite3_free(zErrMsg);
      exit(1);
    }
  }
  log_wallet_info("Created all Database Tables successfully", "");
  return 0;
}