//
// Created by matth on 3/10/2020.
//

#include "../iota-simplewallet.h"

char* get_node_status() {
  return get_config("info");
}