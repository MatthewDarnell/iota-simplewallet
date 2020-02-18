//
// Created by matth on 2/18/2020.
//

#include <stdio.h>
#include "input_to_output.h"

int create_input_to_output(sqlite3* db, const char* input_hash, int64_t output_serial) {
  sqlite3_stmt* stmt;
  int rc;

  char* query = "INSERT INTO input_to_output(input, output) VALUES(?, ?)";
  rc = sqlite3_prepare_v2(db, query, -1, &stmt, 0);

  if (rc != SQLITE_OK) {
    fprintf(stderr, "%s -- Failed to create prepared statement: %s\n", __func__, sqlite3_errmsg(db));
    return -1;
  }

  sqlite3_bind_text(stmt, 1, input_hash, -1, NULL);
  sqlite3_bind_int64(stmt, 2, output_serial);

  rc = sqlite3_step(stmt);

  if (rc != SQLITE_DONE) {
    fprintf(stderr,"%s execution failed: %s\n", __func__, sqlite3_errmsg(db));
    sqlite3_reset(stmt);
    sqlite3_finalize(stmt);
    return -1;
  }
  sqlite3_finalize(stmt);
#ifdef WIN32
  fprintf(stdout, "Created new input->output link <%s> (%I64d i)\n", input_hash, output_serial);
#else
  fprintf(stdout, "Created new outgoing transaction to address <%s> (%lld i)\n", input, output_serial);
#endif
  return 0;
}

cJSON* get_inputs_for_output(sqlite3* db, int64_t output_serial) {
  sqlite3_stmt* stmt;
  int rc;

  char* query = "SELECT * FROM input_to_output WHERE output=?";
  rc = sqlite3_prepare_v2(db, query, -1, &stmt, 0);

  if (rc != SQLITE_OK) {
    fprintf(stderr, "%s -- Failed to create prepared statement: %s\n", __func__, sqlite3_errmsg(db));
    return NULL;
  }
  sqlite3_bind_int64(stmt, 1, output_serial);

  rc = sqlite3_step(stmt);

  cJSON *json = cJSON_CreateArray();

  while(rc == SQLITE_ROW) {
    cJSON *row =  cJSON_CreateObject();

    cJSON_AddNumberToObject(row, "serial", sqlite3_column_int(stmt, 0 ));

    cJSON_AddStringToObject(row, "input", (char*)sqlite3_column_text(stmt, 1 ));
    char str_output_serial[64] = { 0 };
#ifdef WIN32
    snprintf(str_output_serial, 64, "%I64d", output_serial);
#else
    snprintf(str_output_serial, 64, "%lld", output_serial);
#endif
    cJSON_AddStringToObject(row, "output", str_output_serial);

    cJSON_AddItemToArray(json, row);
    rc = sqlite3_step(stmt);
  }

  sqlite3_finalize(stmt);
  return json;
}