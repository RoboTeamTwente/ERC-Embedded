
#include "logging.h"

#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cubemx_main.h"
#include "stm32h7xx_hal_def.h"

static UART_HandleTypeDef huart_handler;
// Char=byte, system doesnt seem to have bool;
static char initialized = 0;
static HAL_StatusTypeDef hstatus;

/**
 * @brief  Retargets the C library printf function to the USART.
 * @param  file: File descriptor.
 * @param  ptr: Pointer to the data to be written.
 * @param  len: Length of the data.
 * @retval The number of characters written.
 */
int _write(int file, char* ptr, int len) {
    // Only handle stdout (standard output), which has a file descriptor of 1
    if (file == 1 || file == 2) {
        // Transmit the character buffer 'ptr' of length 'len' via UART
        // The last parameter is the timeout in milliseconds.
        hstatus = HAL_UART_Transmit(&huart_handler, (uint8_t*)ptr, len,
                                    HAL_MAX_DELAY);

        return hstatus == HAL_OK ? len : -1;
    }
    return -1;
}

/**
 * @param arg a pointer to a UART_HandleTypeDef
 */
void LOG_init(void* arg) {
    huart_handler = *(UART_HandleTypeDef*)arg;
    initialized = 1;
    printf("\n\n\n[BOOT] ########========--------\n");
    LOG(LOG_INFO, "LOGGING", "Logging Initialized");
}

void LOG(LogLevel level, const char* TAG, const char* log_message, ...) {
    if (initialized == 0) {
        // Early return in case huart_handler is not set;
        return;
    }
    const char* level_str = "UNKNOWN";  // Default
    if (level >= LOG_INFO && level <= LOG_ERROR) {
        level_str = LOG_LEVEL_STRINGS[level];
    }
    printf("[%s] %s: ", level_str, TAG);

    va_list args;
    va_start(args, log_message);

    vprintf(log_message, args);

    va_end(args);

    printf("\n");
}
