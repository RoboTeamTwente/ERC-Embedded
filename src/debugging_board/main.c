#include <stddef.h>
#include <stdio.h>

#include "components/common/envelope.pb.h"
#include "components/sensor_board/gps_sensor.pb.h"
#include "components/sensor_board/ph_sensor.pb.h"
#include "decoding_task.h"
#include "portmacro.h"
#include "stm/ethernet_udp.h"
#ifndef UNIT_TEST
#include "FreeRTOS.h"
#include "bucketed_pqueue.h"
#include "cmsis_os2.h"  // FreeRTOS wrapper header (v2)
#include "cubemx_main.h"
#include "gpio.h"
#include "ili9341.h"
#include "ili9341_fonts.h"
#include "logging.h"
#include "menu_driver.h"
#include "menu_driver_icons.h"
#include "menu_driver_imgs.h"
#include "menu_driver_list.h"
#include "pb_message.h"
#include "result.h"
#include "string.h"
#include "task.h"

static char* TAG = "MAIN";

void Error_Handler(void);
void cubemx_main(void);
void SystemClock_Config(void);
void MPU_Config(void);
void MX_GPIO_Init(void);

COM_InitTypeDef BspCOMInit;
UART_HandleTypeDef huart_com;
void MainTask(void* argument);

// Task attributes for CMSIS-RTOS v2
const osThreadAttr_t mainTask_attributes = {
    .name = "mainTask",
    .stack_size = 1024 * 2,
    .priority = (osPriority_t)tskIDLE_PRIORITY + 1U,
};
SPI_HandleTypeDef hspi1;
extern s_menu_driver_task_handle;

static result_t HandleType1Packet(void* buffer) {
    if (buffer == NULL) {
        return RESULT_ERR_INVALID_ARG;
    }

    SensorBoardGPSInfo* packet = (SensorBoardGPSInfo*)buffer;
    printf("Envelope of type gps info has value: %f\n", packet->speed);
    return RESULT_OK;
}

static result_t HandleType2Packet(void* buffer) {
    if (buffer == NULL) {
        return RESULT_ERR_INVALID_ARG;
    }
    SensorBoardPHInfo* packet = (SensorBoardPHInfo*)buffer;
    printf("envelope of type ph info has value: %f\n", packet->ph_value);
    return RESULT_OK;
}

#define DISPATCHER_INPUT_QUEUE_LENGTH 8U
#define HANDLER_QUEUE_LENGTH 4U
#define PRODUCER_TASK_STACK_DEPTH 768U

#define configCHECK_FOR_STACK_OVERFLOW 2
static QueueHandle_t g_dispatcher_input_queue;
static QueueHandle_t g_type1_queue;
static QueueHandle_t g_type2_queue;

static uint8_t packet1_payload[PBEnvelope_size] = {
    0x62, 0x2C, 0x09, 0x13, 0xF2, 0x41, 0xCF, 0x66, 0x1D, 0x4A, 0x40, 0x11,
    0x2C, 0x65, 0x19, 0xE2, 0x58, 0x97, 0x1B, 0x40, 0x1D, 0x00, 0x00, 0x0C,
    0x42, 0x2D, 0x00, 0x00, 0x87, 0x43, 0x35, 0x9A, 0x99, 0x99, 0x3F, 0x3D,
    0x66, 0x66, 0xE6, 0x3F, 0x40, 0x09, 0x48, 0x01, 0x50, 0x01};
static receive_frame packet1 = {.payload = packet1_payload,
                                .len = sizeof(packet1_payload)};

static uint8_t packet2_payload[PBEnvelope_size] = {

    0x0A, 0x11, 0x0D, 0x66, 0x66, 0xE6, 0x40, 0x15, 0x00, 0x00,
    0x00, 0x44, 0x1D, 0x00, 0x00, 0xAC, 0x41, 0x20, 0x01};
static receive_frame packet2 = {.payload = packet2_payload,
                                .len = sizeof(packet2_payload)};

static packet_dispatcher_config_t disp_conf = {
    .task_count = 2,
    .dispatcher_priority = tskIDLE_PRIORITY + 3U,
    .dispatcher_stack_depth = 1 * 1024U};

void MainTask(void* argument) {
    LOGI(TAG, "In main");
    g_dispatcher_input_queue =
        xQueueCreate(DISPATCHER_INPUT_QUEUE_LENGTH, sizeof(receive_frame));
    if (g_dispatcher_input_queue == NULL) {
        LOGE(TAG, "Could not create dispatcher input queue");
        return;
    }
    LOGI(TAG, "created dispatcher queue: %p", (void*)g_dispatcher_input_queue);

    static packet_handler_config_t handler_configs[2];
    handler_configs[0].handler = HandleType1Packet;
    handler_configs[0].task_name = "PktType1";
    handler_configs[0].packet_type = PBEnvelope_gps_info_tag;
    handler_configs[0].item_size = SensorBoardGPSInfo_size;
    handler_configs[0].task_priority = tskIDLE_PRIORITY + 2U;
    handler_configs[0].queue_length = 5;
    // handler_configs[0].queue_buffer = packet1_buffer;
    // handler_configs[0].task_stack_depth = 512U;

    handler_configs[1].handler = HandleType2Packet;
    handler_configs[1].task_name = "PktType2";
    handler_configs[1].packet_type = PBEnvelope_ph_info_tag;
    handler_configs[1].item_size = SensorBoardPHInfo_size;
    handler_configs[1].task_priority = tskIDLE_PRIORITY + 2U;
    handler_configs[1].queue_length = 5;
    // handler_configs[1].queue_buffer = packet2_buffer;
    // handler_configs[1].task_stack_depth = 512U;

    disp_conf.tasks = handler_configs;
    disp_conf.input_queue = g_dispatcher_input_queue;

    LOGI(TAG, "handlers configured");

    BaseType_t ok;
    PacketDispatcherStart(&disp_conf);

    LOGI(TAG, "Packet 1 p: %p", (void*)packet1_payload);
    LOGI(TAG, "Packet 2 p: %p", (void*)packet2_payload);
    for (;;) {
        LOGI(TAG, "Sending packets");

        ok = xQueueSend(g_dispatcher_input_queue, &packet1, portMAX_DELAY);
        LOGI(TAG, "Sent packet1: %ld", (long)ok);

        osDelay(1);

        ok = xQueueSend(g_dispatcher_input_queue, &packet2, portMAX_DELAY);
        LOGI(TAG, "Sent packet2: %ld", (long)ok);
        osDelay(1);
    }
}

void main() {
    SCB_EnableICache();
    SCB_EnableDCache();

    HAL_Init();

    MX_GPIO_Init();
    SystemClock_Config();
    MX_SPI1_Init();

    HAL_GPIO_WritePin(LCD_CS_PORT, LCD_CS_PIN, GPIO_PIN_RESET);  // CS OFF

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

    osKernelInitialize();
    LOGI(TAG, "Kernel Initialized");
    osThreadNew(MainTask, NULL, &mainTask_attributes);
    // menu_driver_task_spawn();
    osKernelStart();
    while (1) {
    }
}

#endif  //! UNIT_TEST
//
