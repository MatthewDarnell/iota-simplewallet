//
// Created by matth on 2/19/2020.
//

#include <utils/logger_helper.h>
#include <cclient/api/core/logger.h>
#include <cclient/api/extended/logger.h>
#include <cclient/serialization/json/logger.h>
#include "init.h"

void init_iota() {
  logger_helper_init(0);
  logger_init_client_core(LOGGER_DEBUG);
  logger_init_client_extended(LOGGER_DEBUG);
  logger_init_json_serializer(LOGGER_DEBUG);
}