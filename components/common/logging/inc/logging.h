#ifndef LOGGING_H
#define LOGGING_H

#include <assert.h>
static const char *LOG_LEVEL_STRINGS[] = {
    "INFO",
    "WARNING",
    "ERROR",
};

typedef enum {
  LOG_INFO,
  LOG_WARNING,
  LOG_ERROR,
  _LOG_LAST_LEVEL_DONT_EDIT
} LogLevel;

static inline const char *log_level_to_string(LogLevel logLevel) {
  static_assert((sizeof(LOG_LEVEL_STRINGS) / sizeof(LOG_LEVEL_STRINGS[0])) ==
                    _LOG_LAST_LEVEL_DONT_EDIT,
                "Mismatch in number of log level strings!");

  if (logLevel >= 0 && logLevel < _LOG_LAST_LEVEL_DONT_EDIT) {
    return LOG_LEVEL_STRINGS[logLevel];
  }
  return "NoLevel";
}

/**
 * @brief initializes the LOG library, it should only occur once throughout the
 * program
 */
void LOG_init(void *arg);

/**
 * @brief Logs over UART like printf logs
 *
 * @param TAG The tag representing from where the log is send
 * @param log_message The message to be send
 * @param ... The variables to be added to the log message
 */
void LOG(LogLevel level, const char *TAG, const char *log_message, ...);

// if config_log_level not defined from the compiler, use log level info
#ifndef CONFIG_LOG_LEVEL
#define CONFIG_LOG_LEVEL LOG_INFO
#endif

#if (CONFIG_LOG_LEVEL <= LOG_ERROR)
#define LOGE(TAG, format, ...) LOG(LOG_ERROR, TAG, format, ##__VA_ARGS__)
#else
#define LOGE(TAG, format, ...) (void)0
#endif

// WARNING Level Macro
#if (CONFIG_LOG_LEVEL <= LOG_WARNING)
#define LOGW(TAG, format, ...) LOG(LOG_WARNING, TAG, format, ##__VA_ARGS__)
#else
#define LOGW(TAG, format, ...) (void)0
#endif

// INFO Level Macro
#if (CONFIG_LOG_LEVEL <= LOG_INFO)
#define LOGI(TAG, format, ...) LOG(LOG_INFO, TAG, format, ##__VA_ARGS__)
#else
#define LOGI(TAG, format, ...) (void)0
#endif

#endif
