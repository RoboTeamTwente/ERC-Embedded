#ifndef RESULT_H
#define RESULT_H

#include <stdio.h>

typedef enum {
  RESULT_OK = 0, // On successful operations

  // --- Generic Failure ---
  RESULT_FAIL, // Generic failure, pick this if none other fit

  // --- Specific Errors ---
  RESULT_ERR_ACCESS_DENIED,  // Operation not permitted (e.g., file permissions)
  RESULT_ERR_ALREADY_EXISTS, // The resource to be created already exists
  RESULT_ERR_BAD_FORMAT,     // Data is corrupt or in an unexpected format
  RESULT_ERR_BUSY,           // The resource is busy, try again later
  RESULT_ERR_COMMS,          // A communication error occurred (e.g., UART, I2C)
  RESULT_ERR_INVALID_ARG,    // Invalid arguments on input
  RESULT_ERR_INVALID_STATE,  // The system is in a state that forbids the
                             // operation
  RESULT_ERR_IO,             // A general-purpose Input/Output error
  RESULT_ERR_NOT_FOUND,      // A requested resource could not be found
  RESULT_ERR_NOT_INITIALIZED, // A required module or resource has not been
                              // initialized
  RESULT_ERR_NO_MEM,          // Out of memory
  RESULT_ERR_OVERFLOW, // An operation resulted in an overflow (arithmetic or
                       // buffer)
  RESULT_ERR_TIMEOUT,  // The operation did not complete in the expected time
  RESULT_ERR_UNIMPLEMENTED, // Function/operation is not implemented
} result_t;

/**
 * @brief  Converts a result code to a short, human-readable string.
 * @param  code The result_t code to convert.
 * @return A constant string literal representing the code.
 */
const char *result_to_short_str(result_t code);

/**
 * @brief  Converts a result code to a longer, descriptive string.
 * @param  code The result_t code to convert.
 * @return A constant string literal explaining the code.
 */
const char *result_to_desc_str(result_t code);

/**
 * @brief Macro that evaluates an expression and has an early return if the
 * expression does not evaluate to RESULT_OK.
 */
#define TRY(expr)                                                              \
  do {                                                                         \
    result_t _try_status = (expr);                                             \
    if (_try_status != RESULT_OK) {                                            \
      return _try_status;                                                      \
    }                                                                          \
  } while (0)

/**
 * @brief Macro that evaluates an expression and goes to cleanup lavel if the
 * expression does not evaluate to REUSLT_OK
 */
#define TRY_CLEAN(expr)                                                        \
  do {                                                                         \
    result_t _try_status = (expr);                                             \
    if (_try_status != RESULT_OK) {                                            \
      goto cleanup;                                                            \
    }                                                                          \
  } while (0)

#ifdef LOGGING_H

#ifdef TAG
// --- TAG is defined, so we can use it in the log messages. ---

/**
 * @brief If logging is enabled: Checks, logs on failure using TAG, and returns.
 */
#define TRY_LOG(expr)                                                          \
  do {                                                                         \
    result_t _try_status = (expr);                                             \
    if (_try_status != RESULT_OK) {                                            \
      LOGE(TAG, "%s: %s", result_to_short_str(_try_status),                    \
           result_to_desc_str(_try_status));                                   \
      return _try_status;                                                      \
    }                                                                          \
  } while (0)

/**
 * @brief If logging is enabled: Checks, logs on failure using TAG, and jumps to
 * cleanup.
 */
#define TRY_LOG_CLEAN(expr)                                                    \
  do {                                                                         \
    result_t _try_status = (expr);                                             \
    if (_try_status != RESULT_OK) {                                            \
      LOGE(TAG, "%s: %s", result_to_short_str(_try_status),                    \
           result_to_desc_str(_try_status));                                   \
      goto cleanup;                                                            \
    }                                                                          \
  } while (0)

#else
#if defined(__GNUC__) || defined(__clang__)
// For GCC and Clang, we can use _Pragma to inject a warning directly into the
// macro.
#define GENERATE_WARNING(msg) _Pragma(msg)
#define TRY_LOG(expr)                                                          \
  do {                                                                         \
    GENERATE_WARNING(                                                          \
        "GCC warning \"TRY_LOG used but LOGGING_H is not defined.\"");         \
    TRY(expr);                                                                 \
  } while (0)
#define TRY_LOG_CLEAN(expr)                                                    \
  do {                                                                         \
    GENERATE_WARNING(                                                          \
        "GCC warning \"TRY_LOG_CLEAN used but LOGGING_H is not defined.\"");   \
    TRY_CLEAN(expr);                                                           \
  } while (0)

#elif defined(_MSC_VER)
// For MSVC, the syntax is __pragma(message(...))
#define TRY_LOG(expr)                                                          \
  do {                                                                         \
    __pragma(message("Warning: TRY_LOG used but LOGGING_H is not defined."));  \
    TRY(expr);                                                                 \
  } while (0)
#define TRY_LOG_CLEAN(expr)                                                    \
  do {                                                                         \
    __pragma(                                                                  \
        message("Warning: TRY_LOG_CLEAN used but LOGGING_H is not defined.")); \
    TRY_CLEAN(expr);                                                           \
  } while (0)

#else
// For other compilers, issue a single, general warning when this header is
// included.
#warning                                                                       \
    "Logging is disabled; TRY_LOG and TRY_LOG_CLEAN will not generate log output."
#define TRY_LOG(expr) TRY(expr)
#define TRY_LOG_CLEAN(expr) TRY_CLEAN(expr)
#endif
#endif
#endif
#endif
