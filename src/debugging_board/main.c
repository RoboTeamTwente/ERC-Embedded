#ifndef UNIT_TEST
//
// #include <stddef.h>
// #include <stdio.h>
//
#include "FreeRTOS.h"
// #include "bucketed_pqueue.h"
#include "cmsis_os2.h"  // FreeRTOS wrapper header (v2)
// #include "components/common/envelope.pb.h"
// #include "components/common/packet_dispatcher/packet_dispatcher.h"
// #include "components/sensor_board/gps_sensor.pb.h"
// #include "components/sensor_board/ph_sensor.pb.h"
#include "cubemx_main.h"
#include "gpio.h"
// #include "tim.h"
// #include "ili9341.h"
#include "logging.h"
// #include "packet_dispatcher.h"
// #include "packet_dispatcher_macros.h"
// #include "portmacro.h"
#include "result.h"
// #include "spi.h"
// #include "stm/ethernet_udp.h"
// #include "string.h"
#include "ethernet.h"
#include "networking_constants.h"
#include "task.h"
//
static char* TAG = "MAIN";
//
// void Error_Handler(void);
// void cubemx_main(void);
// void SystemClock_Config(void);
// void MPU_Config(void);
// void MX_GPIO_Init(void);
//
COM_InitTypeDef BspCOMInit;
UART_HandleTypeDef huart_com;
// void MainTask(void* argument);
//
// // Task attributes for CMSIS-RTOS v2
const osThreadAttr_t mainTask_attributes = {
    .name = "mainTask",
    .stack_size = 1024 * 2,
    .priority = (osPriority_t)osPriorityNormal,
};
// SPI_HandleTypeDef hspi1;
// extern s_menu_driver_task_handle;
//
// static result_t HandleType1Packet(void* buffer) {
//     if (buffer == NULL) {
//         return RESULT_ERR_INVALID_ARG;
//     }
//
//     SensorBoardGPSInfo* packet = (SensorBoardGPSInfo*)buffer;
//     printf("Envelope of type gps info has value: %f\n", packet->speed);
//     return RESULT_OK;
// }
//
// static result_t HandleType2Packet(void* buffer) {
//     if (buffer == NULL) {
//         return RESULT_ERR_INVALID_ARG;
//     }
//     SensorBoardPHInfo* packet = (SensorBoardPHInfo*)buffer;
//     printf("envelope of type ph info has value: %f\n", packet->ph_value);
//     return RESULT_OK;
// }
//
// #define DISPATCHER_INPUT_QUEUE_LENGTH 8U
// // #define HANDLER_QUEUE_LENGTH 4U
// // #define PRODUCER_TASK_STACK_DEPTH 768U
// //
// // #define configCHECK_FOR_STACK_OVERFLOW 2
// static QueueHandle_t g_dispatcher_input_queue;
// // static QueueHandle_t g_type1_queue;
// // static QueueHandle_t g_type2_queue;
//
// static uint8_t packet1_payload[] = {
//     0x62, 0x2C, 0x09, 0x13, 0xF2, 0x41, 0xCF, 0x66, 0x1D, 0x4A, 0x40, 0x11,
//     0x2C, 0x65, 0x19, 0xE2, 0x58, 0x97, 0x1B, 0x40, 0x1D, 0x00, 0x00, 0x0C,
//     0x42, 0x2D, 0x00, 0x00, 0x87, 0x43, 0x35, 0x9A, 0x99, 0x99, 0x3F, 0x3D,
//     0x66, 0x66, 0xE6, 0x3F, 0x40, 0x09, 0x48, 0x01, 0x50, 0x01};
// static receive_frame_t packet1 = {.payload = packet1_payload,
//
//                                   .len = sizeof(packet1_payload)};
//
// static uint8_t packet2_payload[] = {
//
//     0x0A, 0x11, 0x0D, 0x66, 0x66, 0xE6, 0x40, 0x15, 0x00, 0x00,
//     0x00, 0x44, 0x1D, 0x00, 0x00, 0xAC, 0x41, 0x20, 0x01};
// static receive_frame_t packet2 = {.payload = packet2_payload,
//                                   .len = sizeof(packet2_payload)};
// static uint8_t packet1_buffer[SensorBoardGPSInfo_size * 5];
// static uint8_t packet2_buffer[SensorBoardPHInfo_size * 5];
//
// // static packet_handler_config_t handler_configs[] = {
// //     {.handler = HandleType1Packet,
// //      .task_name = "GPS Handler",
// //      .packet_type = PBEnvelope_gps_info_tag,
// //      .item_size = SensorBoardGPSInfo_size,
// //      .task_priority = tskIDLE_PRIORITY + 2U,
// //      .queue_length = 5,
// //      .queue_buffer = packet1_buffer},
// //     {
// //         .handler = HandleType2Packet,
// //         .task_name = "PH Handler",
// //         .packet_type = PBEnvelope_ph_info_tag,
// //         .item_size = SensorBoardPHInfo_size,
// //         .task_priority = tskIDLE_PRIORITY + 2U,
// //         .queue_length = 5,
// //         .queue_buffer = packet2_buffer,
// //     }};
// PACKET_HANDLER_CONFIG_STATIC(gps_handler, PBEnvelope_gps_info_tag, gps_info,
//                              HandleType1Packet);
//
// PACKET_HANDLER_CONFIG_STATIC_QUEUE(ph_handler, PBEnvelope_ph_info_tag,
// ph_info,
//                                    HandleType2Packet, 10);
//
void MainTask(void* argument) {
    LOGI(TAG, "IN MAIN");
    while (1) {
        LOGI(TAG, "IN MAIN");
        osDelay(1000);
    }
}

void main() {
    MPU_Config();

    /* Enable the CPU Cache */

    /* Enable I-Cache---------------------------------------------------------*/
    SCB_EnableICache();

    /* Enable D-Cache---------------------------------------------------------*/
    SCB_EnableDCache();

    /* MCU
     * Configuration--------------------------------------------------------*/

    /* Reset of all peripherals, Initializes the Flash interface and the
     * Systick. */
    HAL_Init();

    /* Configure the system clock */
    SystemClock_Config();

    /* Initialize all configured peripherals */
    MX_GPIO_Init();
    MX_SPI1_Init();

    /* Init scheduler */
    osKernelInitialize(); /* Call init function for freertos objects (in
                             cmsis_os2.c) */
    MX_FREERTOS_Init();

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
    uint8_t mac[6] = NETWORK_MAC;
    uint8_t ip[4] = NETWORK_IP;
    uint8_t netmask[4] = NETMASK;
    uint8_t gateway[4] = GATEWAY;
    ETH_init(ethernet_linkstatus_callback, ip, netmask, gateway, mac);
    int mac1[6] = {0x11, 0x22, 0x33, 0x44, 0x55, 0x66};
    int mac2[6] = {0x12, 0x23, 0x34, 0x45, 0x56, 0x67};
    int mac3[6] = {0x90, 0x2e, 0x16, 0xbe, 0x1b, 0x33};
    ETH_setup_MAC_address_filtering(mac1, mac2, mac3);

    LOGI(TAG, "Kernel Initialized");
    osThreadNew(MainTask, NULL, &mainTask_attributes);
    // menu_driver_task_spawn();
    osKernelStart();
    while (1) {
    }
}
//
#endif  //! UNIT_TEST
