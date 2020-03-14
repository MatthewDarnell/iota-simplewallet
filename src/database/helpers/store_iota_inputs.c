//
// Created by matth on 2/26/2020.
//

#include <stdlib.h>
#include <string.h>
#include "../sqlite3/stores/incoming_transaction.h"
#include "../../iota-simplewallet.h"
#include "../../thread/event_queue.h"
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

      int create_ret_val = 0;
      if(0 == (create_ret_val = create_incoming_transaction(db, address, d_amount, bundle, hash, timestamp, d_confirmed))) {
        char* string = cJSON_PrintUnformatted(transaction);
        push_new_event("transaction_received", string);
        free(string);
      }
      if(create_ret_val > -2) { //Transaction INSERT didn't fail due to db locked issue, in which case we will come back to this tx next thread loop.
                                // May have failed to to unique constraint, in which case let's still mark confirmed.
                                // (create_incoming_transaction returns -2 if db locked causes a failure, otherwise -1 if unique constraint tripped)
        if(d_confirmed > 0) {
          if(0 == mark_incoming_transaction_confirmed(db, hash)) {
            cJSON_DeleteItemFromObject(transaction, "confirmed");
            cJSON_AddNumberToObject(transaction, "confirmed", 1);
            char* string = cJSON_PrintUnformatted(transaction);
            push_new_event("transaction_received_confirmed", string);
            free(string);
          }
        }
      }

    }
  }
  cJSON_Delete(inputs);
  return 0;
}
