#ifndef PIO_UNIT_TESTING
#include "cmsis_os2.h" // FreeRTOS wrapper header (v2)
#include "cubemx_main.h"
#include "gpio.h"
#include "logging.h"
#include "string.h"

#include "imu_sensor.h"
#include "ph_sensor.h"
#include "gps_sensor.h"
#include "sensor_basics.h"

#define MAIN_TASK_DELAY_MS 1000

static char *TAG = "MAIN";

void Error_Handler(void);
void cubemx_main(void);
void SystemClock_Config(void);
void MPU_Config(void);
void MX_GPIO_Init(void);

COM_InitTypeDef BspCOMInit;
UART_HandleTypeDef huart_com;
void MainTask(void *argument);

// Task attributes for CMSIS-RTOS v2
const osThreadAttr_t mainTask_attributes = {
    .name = "mainTask",
    .stack_size = 256 * 4,
    .priority = (osPriority_t)osPriorityNormal,
};

void init_board() {
  HAL_Init();
  SystemClock_Config();

  osKernelInitialize();
  MX_GPIO_Init();

  /* Initialize COM1 port */
  BspCOMInit.BaudRate = 115200;
  BspCOMInit.WordLength = COM_WORDLENGTH_8B;
  BspCOMInit.StopBits = COM_STOPBITS_1;
  BspCOMInit.Parity = COM_PARITY_NONE;
  BspCOMInit.HwFlowCtl = COM_HWCONTROL_NONE;
  if (BSP_COM_Init(COM1, &BspCOMInit) != BSP_ERROR_NONE) {
    Error_Handler();
  }
  MX_USART3_Init(&huart_com, &BspCOMInit);
  LOG_init(&huart_com);

  osThreadNew(MainTask, NULL, &mainTask_attributes);
  osKernelStart();

  while (1) {
  }
}
int main(void) { init_board(); }
/**
 * @brief  Main application task
 * @param  argument: Not used
 * @retval None
 */
void MainTask(void *argument) {
  BSP_LED_Init(LED_GREEN);
  BSP_LED_Init(LED_BLUE);
  BSP_LED_Init(LED_RED);

  // Initialize sensors
  imu_data_t imu_data;
  imu_sensor_init(&imu_data);

  ph_sensor_t ph_sensor;
  ph_sensor_init(&ph_sensor, 3.3f); // 3.3V reference voltage

  gps_data_t gps_data;
  gps_sensor_init(&gps_data);
  

  BSP_LED_Toggle(LED_GREEN);

  while (1) {
    BSP_LED_Toggle(LED_GREEN);
    BSP_LED_Toggle(LED_BLUE);
    BSP_LED_Toggle(LED_RED);

    // Example: Read pH sensor (simulated ADC value)
    float ph_value;
    poll_ph_sensor(&ph_sensor);
    if (ph_sensor_get_value(&ph_sensor, &ph_value) == RESULT_OK) {
      if (validate_ph_value(ph_value) == RESULT_OK) {
        LOGI(TAG, "pH Value: %.2f", ph_value);
      }
    }

    // Example: Update and Log GPS data
    poll_gps_sensor(&gps_data);
    double lat, lon;
    if (gps_sensor_get_coordinates(&gps_data, &lat, &lon) == RESULT_OK) {
      LOGI(TAG, "GPS: Lat: %.6f, Lon: %.6f", lat, lon);
    }
    
    // Example: Log IMU data
    poll_imu_sensor(&imu_data);
    LOGI(TAG, "IMU Accel: [%.2f, %.2f, %.2f]", imu_data.accel[0],
         imu_data.accel[1], imu_data.accel[2]);

    LOGI(TAG, "This is the sensor board");
    osDelay(MAIN_TASK_DELAY_MS);
  }
}

#endif //! PIO_UNIT_TESTS
