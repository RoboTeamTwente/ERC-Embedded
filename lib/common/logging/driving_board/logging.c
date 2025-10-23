#include "logging.h"
#include "cubemx_main.h"
#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static UART_HandleTypeDef huart_handler;
// Char=byte, system doesnt seem to have bool;
static char initialized = 0;

/**
 * @brief  Retargets the C library printf function to the USART.
 * @param  file: File descriptor.
 * @param  ptr: Pointer to the data to be written.
 * @param  len: Length of the data.
 * @retval The number of characters written.
 */
int _write(int file, char *ptr, int len) {
  // Only handle stdout (standard output), which has a file descriptor of 1
  if (file == 1) {
    // Transmit the character buffer 'ptr' of length 'len' via UART
    // The last parameter is the timeout in milliseconds.
    HAL_UART_Transmit(&huart_handler, (uint8_t *)ptr, len, HAL_MAX_DELAY);
  }
  return len;
}

/**
 * @param arg a pointer to a UART_HandleTypeDef
 */
void LOG_init(void *arg) {
  huart_handler = *(UART_HandleTypeDef *)arg;
  initialized = 1;
  LOG(LOG_INFO, "LOGGING", "Logging Initialized");
}

void LOG(LogLevel level, char *TAG, char *log_message, ...) {
  if (initialized == 0) {
    // Early return in case huart_handler is not set;
    return;
  }
  const char *level_str = "UNKNOWN"; // Default
  if (level >= LOG_INFO && level <= LOG_ERROR) {
    level_str = LOG_LEVEL_STRINGS[level];
  }

  // --- Define all string components ---
  const char *punctuation = ": ";
  const char *newline = "\r\n";

  // Calculate the total length needed for the format string
  size_t format_len = strlen("[") + strlen(level_str) + strlen("] ") +
                      strlen(TAG) + strlen(punctuation) + strlen(log_message) +
                      strlen(newline) + 1; // +1 for null terminator

  char *format_message = (char *)malloc(format_len);
  if (format_message == NULL) {
    return; // Malloc failed
  }

  // Build the format string piece by piece
  strcpy(format_message, "[");
  strcat(format_message, level_str);
  strcat(format_message, "] ");
  strcat(format_message, TAG);
  strcat(format_message, punctuation);
  strcat(format_message, log_message);
  strcat(format_message, newline);

  // --- Process variadic arguments and format the final message ---
  va_list args;
  va_start(args, log_message);

  // First, calculate the required size for the final output string
  va_list args_copy;
  va_copy(args_copy, args);
  size_t total_len = vsnprintf(NULL, 0, format_message, args_copy);
  va_end(args_copy);

  if (total_len < 0) {
    free(format_message); // Clean up on error
    va_end(args);
    return;
  }

  // Allocate memory for the final message and create it
  char *total_message = malloc(total_len + 1);
  if (total_message == NULL) {
    free(format_message); // Clean up on error
    va_end(args);
    return;
  }
  vsnprintf(total_message, total_len + 1, format_message, args);
  va_end(args);

  // --- Transmit and clean up ---
  HAL_UART_Transmit(&huart_handler, (uint8_t *)total_message, total_len,
                    HAL_MAX_DELAY);

  free(format_message);
  free(total_message);
}
