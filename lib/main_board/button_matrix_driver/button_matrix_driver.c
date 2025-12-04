#include "button_matrix_driver.h"
#include "cubemx_main.h"
#include "logging.h"
#include "stdint.h"
#include <stdbool.h>

static char *TAG = "BUTTON MATRIX DRIVER";

const static GPIO_TypeDef *row_ports[3] = {
    MATRIX_ROW_A_GPIO_Port, MATRIX_ROW_B_GPIO_Port, MATRIX_ROW_C_GPIO_Port};

const static uint16_t row_pins[3] = {MATRIX_ROW_A_Pin, MATRIX_ROW_B_Pin,
                                     MATRIX_ROW_C_Pin};
const static GPIO_TypeDef *col_ports[3] = {
    MATRIX_COL_A_GPIO_Port, MATRIX_COL_B_GPIO_Port, MATRIX_COL_C_GPIO_Port};
const static uint16_t col_pins[3] = {MATRIX_COL_A_Pin, MATRIX_COL_B_Pin,
                                     MATRIX_COL_C_Pin};

button_matrix_input_t ButtonMatrixDriver_Scan(void) {
  button_matrix_input_t input = {.row = 0, .col = 0, .button_pressed = false};
  for (uint8_t col = 0; col < BUTTON_MATRIX_MAX_COLS; col++) {
    // Set all columns to HIGH
    HAL_GPIO_WritePin((GPIO_TypeDef *)col_ports[col], col_pins[col],
                      GPIO_PIN_RESET);
    for (uint8_t row = 0; row < BUTTON_MATRIX_MAX_ROWS; row++) {
      GPIO_PinState row_state =
          HAL_GPIO_ReadPin((GPIO_TypeDef *)row_ports[row], row_pins[row]);
      if (row_state == GPIO_PIN_RESET) {
        // Button pressed
        LOGI(TAG, "Button matrix input detected: row %d, col %d", row, col);
        input.row = row;
        input.col = col;
      }
    }
    HAL_GPIO_WritePin((GPIO_TypeDef *)col_ports[col], col_pins[col],
                      GPIO_PIN_SET);
  }
  return input;
}
