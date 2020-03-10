//
// Created by matth on 3/10/2020.
//

#ifdef WIN32
#include <windows.h>
#else
#include <unistd.h>
#define Sleep(x) sleep(x/1000)
#endif

#include <pthread.h>
#include <cjson/cJSON.h>
#include "../config/config.h"
#include "../config/logger.h"
#include "../iota/api.h"
#include "node_info_updater.h"


void thread_node_info_updater(void* args) {
  log_wallet_info("Starting Node Info Updater Thread", "");
  int* quit_flag = (int*)args;

  while(1) {
    if (*quit_flag != 0) {
      break;
    }

    cJSON* info = get_node_info();
    if(info) {
      char* str_info = cJSON_PrintUnformatted(info);
      set_config("info", str_info, 0);
      free(str_info);
      cJSON_Delete(info);
    }
    Sleep(30 * 1000);

  }
  log_wallet_info("Shutting Node Info Updater Thread", "");
  pthread_exit(0);
}