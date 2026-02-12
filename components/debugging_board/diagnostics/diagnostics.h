#ifndef DIAGNOSTICS_H
#define DIAGNOSTICS_H
#include "FreeRTOS.h"
#include <stdint.h>

typedef struct {
  TickType_t last_response;
  uint8_t last_code;
  union {
  } last_state;
} diagnostics_t;

#endif // DIAGNOSTICS_H
