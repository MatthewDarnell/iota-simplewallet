//
// Created by matth on 3/14/2020.
//
#include "../database/sqlite3/db.h"
#include "../crypto/crypt.h"
#include "../config/config.h"
#include "../iota/api.h"

int init_iota_simplewallet(const char* data_dir) {
  load_config(data_dir);
  init_db();
  init_crypto();
  init_iota();
  return 0;
}

int shutdown_iota_simplewallet() {
  shutdown_iota();
  shutdown_config();
  return 0;
}