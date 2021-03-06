//
// Created by matth on 2/18/2020.
//

#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <time.h>
#include <pthread.h>
#include <stdlib.h>
#include "../iota-simplewallet.h"

pthread_mutex_t logger_mutex = PTHREAD_MUTEX_INITIALIZER;

void _log(enum LOG_LEVEL lvl, const char* format, ...) {
  char* logger_file = get_config("logFile");
  if(!logger_file) {
    return;
  }
  pthread_mutex_lock(&logger_mutex);

  FILE *oFile = fopen(logger_file, "ab+");
  free(logger_file);
  if(!oFile) {
    pthread_mutex_unlock(&logger_mutex);
    fprintf(stderr, "Unable to open logger file!\n");
    return;
  }

  time_t rawtime;
  struct tm * timeinfo;
  time ( &rawtime );
  timeinfo = localtime ( &rawtime );

  char *level;

  switch (lvl) {
    case 1:
      level = "INFO";
      break;
    case 2:
      level ="DEBUG";
      break;
    case 3:
      level = "ERROR";
      break;
    case 4:
      level = "FATAL";
      break;
    default:
      level = "INFO";
      break;
  };

  char prefix_buffer[64] = { 0 };
  snprintf(prefix_buffer, 63, "%s: %s", level, asctime(timeinfo));
  size_t len = strlen(prefix_buffer);
  prefix_buffer[len-1] = 0; //asctime adds \n

  char stream_buffer[1024] = { 0 };
  va_list args;
  va_start (args, format);
  vsnprintf (stream_buffer, 1024-len-1,format, args);
  va_end (args);

  char out_buffer[1024] = { 0 };
  snprintf(out_buffer, 1024, "%s --- %s\n", prefix_buffer, stream_buffer);
  fwrite(out_buffer, sizeof(char), strlen(out_buffer), oFile);
  fclose(oFile);
  pthread_mutex_unlock(&logger_mutex);

}