#include "result.h"
#include <stdio.h>

/**
 * @brief  Converts a result code to a short, human-readable string.
 * @param  code The result_t code to convert.
 * @return A constant string literal representing the code.
 */
const char *result_to_short_str(result_t code) {
  switch (code) {
  case RESULT_OK:
    return "OK";
  case RESULT_FAIL:
    return "Fail";
  case RESULT_ERR_ACCESS_DENIED:
    return "Access Denied";
  case RESULT_ERR_ALREADY_EXISTS:
    return "Already Exists";
  case RESULT_ERR_BAD_FORMAT:
    return "Bad Format";
  case RESULT_ERR_BUSY:
    return "Busy";
  case RESULT_ERR_COMMS:
    return "Comms Error";
  case RESULT_ERR_INVALID_ARG:
    return "Invalid Argument";
  case RESULT_ERR_INVALID_STATE:
    return "Invalid State";
  case RESULT_ERR_IO:
    return "I/O Error";
  case RESULT_ERR_NOT_FOUND:
    return "Not Found";
  case RESULT_ERR_NOT_INITIALIZED:
    return "Not Initialized";
  case RESULT_ERR_NO_MEM:
    return "Out of Memory";
  case RESULT_ERR_OVERFLOW:
    return "Overflow";
  case RESULT_ERR_TIMEOUT:
    return "Timeout";
  case RESULT_ERR_UNIMPLEMENTED:
    return "Unimplemented";
  default:
    return "Unknown Error";
  }
}

/**
 * @brief  Converts a result code to a longer, descriptive string.
 * @param  code The result_t code to convert.
 * @return A constant string literal explaining the code.
 */
const char *result_to_desc_str(result_t code) {
  switch (code) {
  case RESULT_OK:
    return "The operation completed successfully.";
  case RESULT_FAIL:
    return "A generic, unspecified failure occurred.";
  case RESULT_ERR_ACCESS_DENIED:
    return "Permission was denied to access or modify a resource.";
  case RESULT_ERR_ALREADY_EXISTS:
    return "An attempt was made to create a resource that already exists.";
  case RESULT_ERR_BAD_FORMAT:
    return "Input data was corrupt, malformed, or not in the expected format.";
  case RESULT_ERR_BUSY:
    return "The requested resource is temporarily unavailable. Try again "
           "later.";
  case RESULT_ERR_COMMS:
    return "A hardware communication peripheral (e.g., UART, SPI, I2C) "
           "reported an error.";
  case RESULT_ERR_INVALID_ARG:
    return "A provided argument is null, out of range, or otherwise invalid.";
  case RESULT_ERR_INVALID_STATE:
    return "The operation cannot be performed in the current system or module "
           "state.";
  case RESULT_ERR_IO:
    return "A low-level Input/Output hardware error occurred.";
  case RESULT_ERR_NOT_FOUND:
    return "The requested item (e.g., file, entry, device) could not be found.";
  case RESULT_ERR_NOT_INITIALIZED:
    return "A component or subsystem must be initialized before use.";
  case RESULT_ERR_NO_MEM:
    return "Failed to allocate the required memory from the heap or a pool.";
  case RESULT_ERR_OVERFLOW:
    return "A buffer was too small for the data, or an arithmetic operation "
           "wrapped around.";
  case RESULT_ERR_TIMEOUT:
    return "An operation failed to complete within the allotted time.";
  case RESULT_ERR_UNIMPLEMENTED:
    return "This function exists, but its functionality has not been "
           "implemented yet.";
  default:
    return "An unknown error code was encountered.";
  }
}
