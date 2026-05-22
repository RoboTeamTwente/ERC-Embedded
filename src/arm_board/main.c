/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "cubemx_main.h"
#include "gpio.h"
#include "dma.h"
#include "tim.h"
#include <stdint.h>
#include <stdlib.h>
#include "stepper.h"

#include "logging.h"
#include "result.h"

#include "components/arm_board/movement_control_in.pb.h"
#include "pb_message.h"

#include "cmsis_os.h"
#include "FreeRTOS.h"
#include "task.h"

#define TAG "ARM_BOARD"

extern COM_InitTypeDef BspCOMInit;
extern void SystemClock_Config(void);
extern void MPU_Config_wrapper(void);
extern void MX_DMA_Init(void);

TIM_HandleTypeDef htim2;
UART_HandleTypeDef huart_com;

osThreadId_t pwmScopeTaskHandle;
osThreadId_t stepperTaskHandle;

const osThreadAttr_t pwm_scope_attributes = {
    .name       = "pwm_scope",
    .stack_size = 1024 * 4,
    .priority   = (osPriority_t)osPriorityNormal,
};

const osThreadAttr_t stepper_attributes = {
    .name       = "stepper",
    .stack_size = 1024 * 2,
    .priority   = (osPriority_t)osPriorityNormal,
};

static void pwm_scope_task(void *argument);
static void stepper_task(void *argument);
static void my_BSP_COM_Init(void);

static void my_BSP_COM_Init(void) {
    BspCOMInit.BaudRate   = 115200;
    BspCOMInit.WordLength = COM_WORDLENGTH_8B;
    BspCOMInit.StopBits   = COM_STOPBITS_1;
    BspCOMInit.Parity     = COM_PARITY_NONE;
    BspCOMInit.HwFlowCtl  = COM_HWCONTROL_NONE;

    if (BSP_COM_Init(COM1, &BspCOMInit) != BSP_ERROR_NONE) {Error_Handler();}

    MX_USART3_Init(&huart_com, &BspCOMInit);
}

int main(void) {
    MPU_Config_wrapper();
    HAL_Init();
    SystemClock_Config();

    MX_GPIO_Init();
    MX_DMA_Init();
    MX_TIM2_Init();

    my_BSP_COM_Init();
    LOG_init(&huart_com);

    osKernelInitialize();

    pwmScopeTaskHandle = osThreadNew(pwm_scope_task,NULL,&pwm_scope_attributes);

    if (pwmScopeTaskHandle == NULL) {Error_Handler();}

    stepperTaskHandle = osThreadNew(stepper_task,NULL,&stepper_attributes);

    if (stepperTaskHandle == NULL) {Error_Handler();}

    osKernelStart();

    while (1) {}
}

static void pwm_scope_task(void *argument) {
    (void)argument;

    stepper_t step;
    init_stepper(&step, 1, 50, &htim2);

    static const uint32_t scope_pulse_counts[] = {
        10U, 20U, 50U,
        100U, 200U, 400U, 800U,
        1600U, 3200U, 6400U,
    };

    const size_t count =
        sizeof(scope_pulse_counts) /
        sizeof(scope_pulse_counts[0]);

    for (;;) {
        for (size_t p = 0; p < count; p++) {
            uint32_t pulse_count = scope_pulse_counts[p];

            LOGI(TAG,"--- New pulse count: %lu pulses ---",(unsigned long)pulse_count);

            for (int i = 0; i < 10; i++) {
                LOGI(TAG,"Burst %d/10 — %lu pulses",i + 1,(unsigned long)pulse_count);

                do_pwm_dma(&step, (int)pulse_count);

                while (step.pwm_dma_active) {
                    osDelay(10);
                }
            }
            LOGI(TAG,"Done with %lu pulses, switching...",(unsigned long)pulse_count);
            osDelay(4000U);
        }
    }
}

static void stepper_task(void *argument) {
    (void)argument;

    stepper_t step;
    init_stepper(&step, 1, 50, &htim2);

    for (;;) {
        rotate_stepper(&step, 200);
        osDelay(1000U);
    }
}