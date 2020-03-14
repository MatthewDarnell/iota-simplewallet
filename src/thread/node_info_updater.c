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
#include "../iota/api.h"
#include "../iota-simplewallet.h"
#include "event_queue.h"
#include "node_info_updater.h"

void thread_node_info_updater(void* args) {
  log_wallet_info("Starting Node Info Updater Thread", "");
  int* quit_flag = (int*)args;

  int i;
  while(1) {
    if(*quit_flag != 0) {
      break;
    }

    cJSON* info = get_node_info();
    if(info) {
      char* str_info = cJSON_PrintUnformatted(info);
      push_new_event("node_updated", str_info);
      set_config("info", str_info, 0);
      free(str_info);
      cJSON_Delete(info);
    }

    for(i=0; i < 300; i++) {
      Sleep(100);
      if(*quit_flag != 0) {
        break;
      }
    }
    if(*quit_flag != 0) {
      break;
    }

  }
  log_wallet_info("Shutting Down Node Info Updater Thread", "");
  pthread_exit(0);
}