//
// Created by matth on 2/25/2020.
//

#include <utils/logger_helper.h>
#include <cclient/api/core/logger.h>
#include <cclient/api/extended/logger.h>
#include <cclient/serialization/json/logger.h>
#include "../../config/logger.h"
#include "../api.h"

void init_iota() {
  log_wallet_info("Initializing IOTA C Library", "");
  logger_helper_init(LOGGER_DEBUG);
  logger_init_client_core(LOGGER_DEBUG);
  logger_init_client_extended(LOGGER_DEBUG);
  logger_init_json_serializer(LOGGER_DEBUG);
  init_iota_client();
}
void shutdown_iota() {
  log_wallet_info("Shutting Down IOTA C Library", "");
}