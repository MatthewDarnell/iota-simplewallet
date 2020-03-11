//
// Created by matth on 2/26/2020.
//

#include <stdlib.h>
#include <string.h>
#include "../sqlite3/stores/incoming_transaction.h"
#include "../../iota-simplewallet.h"
#include "store_iota_inputs.h"

int store_inputs(sqlite3* db, char* str_inputs) {
  if(!str_inputs) {
    return -1;
  }
  cJSON* inputs = cJSON_Parse(str_inputs);
  if(!inputs) {
    return -1;
  }
  size_t num_inputs = cJSON_GetArraySize(inputs);
  if(num_inputs < 1) {
    cJSON_Delete(inputs);
    return -1;
  }
  cJSON* input = NULL;
  cJSON_ArrayForEach(input, inputs) {

    if(
      !cJSON_HasObjectItem(input, "transactions")
      ) {
      continue;
    }

    if(
      !cJSON_HasObjectItem(input, "address") ||
      !cJSON_HasObjectItem(input, "balance")
      ) {
      log_wallet_error("Invalid Transaction Input Array for Storing: <%s>", cJSON_Print(input));
      continue;
    }

    const char* address = cJSON_GetObjectItem(input, "address")->valuestring;
    cJSON* transaction_array = cJSON_GetObjectItem(input, "transactions");

    if(!cJSON_IsArray(transaction_array)) {
      log_wallet_error("Invalid Transaction Array for Storing: <%s>", cJSON_Print(input));
      continue;
    }


    cJSON* transaction = NULL;

    cJSON_ArrayForEach(transaction, transaction_array) {
      if(
        !cJSON_HasObjectItem(transaction, "hash") ||
        !cJSON_HasObjectItem(transaction, "bundle") ||
        !cJSON_HasObjectItem(transaction, "amount") ||
        !cJSON_HasObjectItem(transaction, "timestamp") ||
        !cJSON_HasObjectItem(transaction, "confirmed")
        ) {
        log_wallet_error("Invalid Transaction Input for Storing: <%s>", cJSON_Print(transaction));
        continue;
      }
      const char* hash = cJSON_GetObjectItem(transaction, "hash")->valuestring;
      const char* bundle = cJSON_GetObjectItem(transaction, "bundle")->valuestring;
      const char* amount = cJSON_GetObjectItem(transaction, "amount")->valuestring;
      const char* timestamp = cJSON_GetObjectItem(transaction, "timestamp")->valuestring;
      const char* confirmed = cJSON_GetObjectItem(transaction, "confirmed")->valuestring;

      uint64_t d_amount = strtoull(amount, NULL, 10);

      int d_confirmed = (strcasecmp(confirmed, "true") == 0) ? 1 : 0;

      create_incoming_transaction(db, address, d_amount, bundle, hash, timestamp, d_confirmed);

    }
  }
  cJSON_Delete(inputs);
  return 0;
}
