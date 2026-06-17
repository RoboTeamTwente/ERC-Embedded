#ifndef MAIN_H
#define MAIN_H

#include "can.h"
#include "ip_mac_constants.h"
#include <stdint.h>

#include "erc-control-arm/control_arm_manual_ert_rtw/control_arm_manual.h"

#include "stepper.h"
#include "tim.h"
// freertos
#include "FreeRTOS.h"
#include "queue.h"

#define calibration 0 // 0 = off, 1 = left, 2 = right, 3 = both

#define STEPPER_QUEUE_SIZE sizeof(arm_stepper_signals)
#define STEPPER_QUEUE_LENGTH 5

typedef struct {
  uint32_t stepper_steps;
  uint32_t stepper_freq;
} arm_stepper_signals;

extern ExtY rtY; // Get controls in :)
extern ExtU rtU; // Get controls in :)
extern void controlArmManualStep(void);
/*External functions*/
extern COM_InitTypeDef BspCOMInit;
extern void MX_FREERTOS_Init(void);
extern void SystemClock_Config(void);
extern void MPU_Config_wrapper(void);
extern void MX_DMA_Init(void);

// IPs
// MY LAPTOP
uint8_t my_mac[6] = {0x6c, 0x24, 0x08, 0xd2, 0xfa, 0x50};
uint8_t my_ip[4] = {192, 168, 0, 111};
uint8_t netmask[4] = NETMASK;
uint8_t gateway[4] = GATEWAY;
// OTHER LAPTOP
uint8_t ip[4] = {192, 168, 0, 50};
uint8_t mac[6] = NETWORK_MAC;

// timer objects
TIM_HandleTypeDef htim2;
TIM_HandleTypeDef htim3;

// huart object
UART_HandleTypeDef huart_com;

// Stepper objects
stepper_t stepperLeft;
stepper_t stepperRight;

QueueHandle_t xQueueStepperLeft;
QueueHandle_t xQueueStepperRight;

// static uint8_t xQueueStepper1Storage[STEPPER_QUEUE_SIZE * STEPPER_QUEUE_LENGTH];
// static StaticQueue_t xQueueStepper1QueueBuffer;
// static uint8_t xQueueStepper2Storage[STEPPER_QUEUE_SIZE * STEPPER_QUEUE_LENGTH];
// static StaticQueue_t xQueueStepper2QueueBuffer;
// QueueHandle_t xQueueStepper3;
// static uint8_t xQueueStepper3Storage[STEPPER_QUEUE_SIZE * STEPPER_QUEUE_LENGTH];
// static StaticQueue_t xQueueStepper3QueueBuffer;

static void vEthernetTask(void *argument);
static void vStepperTaskLeft(void *argument);
static void vStepperTaskRight(void *argument);
static void vArmInTask(void *argument);
static void vArmController(void *argument);
static void vWristController(void *argument);
static void pwm_scope_task(void *argument);

#endif // !MAIN_H
