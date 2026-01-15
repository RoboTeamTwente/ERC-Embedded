#include "logging.h"
#include <stdarg.h>
#include <stdio.h>

void LOG_init(void *arg) {}

void LOG(LogLevel level, char *TAG, char *log_message, ...) {
  fprintf(stderr, "[%s] %s: ", TAG, log_level_to_string(level));
  va_list args;
  va_start(args, log_message);
  fprintf(stderr, log_message, args);
  fprintf(stderr, "\n");
}
