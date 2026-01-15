#ifndef BUTTON_MATRIX_DRIVER_H__
#define BUTTON_MATRIX_DRIVER_H__

#include <stdbool.h>
#include <stdint.h>

#define BUTTON_MATRIX_MAX_ROWS 3
#define BUTTON_MATRIX_MAX_COLS 3
#define BUTTON_MATRIX_TOTAL_BUTTONS                                            \
  (BUTTON_MATRIX_MAX_ROWS * BUTTON_MATRIX_MAX_COLS)

typedef struct {
  uint8_t row;
  uint8_t col;
  bool button_pressed;
} button_matrix_input_t;

button_matrix_input_t ButtonMatrixDriver_Scan(void);

#endif
