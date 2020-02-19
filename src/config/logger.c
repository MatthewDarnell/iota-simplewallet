//
// Created by matth on 2/18/2020.
//

#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>
#include "config.h"
#include "logger.h"

void _log(enum LOG_LEVEL lvl, const char* format, ...) {
  char* logger_file = get_config("logFile");
  if(!logger_file) {
    return;
  }
  FILE *oFile = fopen(logger_file, "ab+");
  free(logger_file);
  if(!oFile) {
    fprintf(stderr, "Unable to open logger file!\n");
    return;
  } else {
  }
  time_t rawtime;
  struct tm * timeinfo;
  time ( &rawtime );
  timeinfo = localtime ( &rawtime );

  char *level;
  switch (lvl) {
    case 1:
      level = "INFO";
    case 2:
      level = "DEBUG";
    case 3:
      level = "ERROR";
    case 4:
      level = "FATAL";
    default:
      level = "INFO";
  };

  char prefix_buffer[32] = { 0 };
  snprintf(prefix_buffer, 32, "%s: %s", level, asctime(timeinfo));
  size_t len = strlen(prefix_buffer);
  prefix_buffer[len-1] = 0; //asctime adds \n

  char stream_buffer[512] = { 0 };
  va_list args;
  va_start (args, format);
  vsnprintf (stream_buffer, 512-len-1,format, args);
  va_end (args);

  char out_buffer[512] = { 0 };
  snprintf(out_buffer, 512, "%s --- %s\n", prefix_buffer, stream_buffer);
  fwrite(out_buffer, sizeof(char), strlen(out_buffer), oFile);
  fclose(oFile);
}