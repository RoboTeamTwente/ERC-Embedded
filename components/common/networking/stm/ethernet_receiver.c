#include "ethernet_receiver.h"
#include "FreeRTOS.h"
#include "FreeRTOSConfig.h"
#include "bucketed_pqueue.h"
#include "cmsis_os2.h"
#include "logging.h"
#include "netif.h"
#include "networking_constants.h"
#include "pbuf.h"
#include "queue.h"
#include "result.h"
#include "stm32h7xx_hal_eth.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define TAG "etherent_receiver"
#define LWIP_HOOK_UNKNOWN_ETH_PROTOCOL(pbuf, netif) eth_reader(netif, pbuf)

extern ETH_HandleTypeDef heth;

receiver_callback r_callback;
// bucketed_pqueue_t ETH_receiver_queue;
// static StaticQueue_t xStaticQueue;
// uint8_t ucQueueStorageArea[ETHERNET_RQ_LENGTH * ETHERNET_RQ_ITEM_SIZE];

// #define STACK_SIZE 200
// StaticTask_t xTaskBuffer;
// StackType_t xStack[STACK_SIZE];

u8_t eth_reader(struct netif *netif, struct pbuf *p) {
  // result_t err = RESULT_OK;
  // struct pbuf *copy_pbuf = pbuf_alloc(PBUF_RAW, p->tot_len, PBUF_RAM);
  // if (copy_pbuf != NULL) {
  //   if (pbuf_take(copy_pbuf, p->payload, p->tot_len) == ERR_OK) {
  //     err = bucketed_pqueue_push(&ETH_receiver_queue, 0, copy_pbuf, 0U);
  //   } else {
  //     pbuf_free(copy_pbuf);
  //     err = RESULT_ERR_BUFF;
  //   }
  // } else {
  //   err = RESULT_ERR_BUFF;
  // }
  // if (err != RESULT_OK) {
  //   LOGE(TAG, "Could not push incomming message to queue");
  // }
  return 1; // not handled, we never handle it, because I have no clue what I am
            // doing
}

/**
 * @brief creates a hexstring from bytes
 *
 * @param payload the byte array
 * @param length the lenght of the byte array
 *
 * @return the hex string
 */
char *bytes_to_hex_string(void *payload, size_t length) {
  uint8_t *bytes = (uint8_t *)payload;
  // Each byte becomes two hex characters, plus null terminator
  char *hex_str = malloc(length * 2 + 1);
  if (!hex_str)
    return NULL;

  for (size_t i = 0; i < length; ++i) {
    sprintf(&hex_str[i * 2], "%02X", bytes[i]);
  }
  return hex_str;
}

void ETH_receiver_callback_example(void *payload, size_t length) {

  char *data = bytes_to_hex_string(payload, length);

  LOGI(TAG, "DATA RECEIVED: %s\n", data);
  LOGI(TAG, "DMA ERROR CODE: %d\n", heth.DMAErrorCode);
  LOGI(TAG, "ERROR CODE: %d\n", heth.ErrorCode);
  free(data);
}

void ETH_set_receiver_callback(receiver_callback callback) {
  if (callback != NULL) {
    r_callback = callback;
  } else {
    r_callback = ETH_receiver_callback_example;
  }
}

// void ETH_receiver_task(void *pvParameters) {

//   configASSERT((uint32_t)pvParameters == 1UL);

//   for (;;) {
//     (void)ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
//     for (;;) {
//       struct pbuf p;
//       result_t r = bucketed_pqueue_pop(&ETH_receiver_queue, &p);
//       r_callback(p.payload, p.len);
//       if (r != RESULT_OK) {
//         break;
//       }
//     }
//   }
// }
result_t ETH_receiver_init(receiver_callback callback) {

  // QueueHandle_t queue =
  //     xQueueCreateStatic(ETHERNET_RQ_LENGTH, ETHERNET_RQ_ITEM_SIZE,
  //                        ucQueueStorageArea, &xStaticQueue);

  // TaskHandle_t xHandle = NULL;
  // xHandle = xTaskCreateStatic(

  //     ETH_receiver_task, /* Function that implements the task. */

  //     "Ethernet receiver task", /* Text name for the task. */

  //     STACK_SIZE, /* Number of indexes in the xStack array. */

  //     (void *)1, /* Parameter passed into the task. */

  //     tskIDLE_PRIORITY, /* Priority at which the task is created. */

  //     xStack, /* Array to use as the task's stack. */

  //     &xTaskBuffer); /* Variable to hold the task's data structure. */

  // if (queue == NULL || xHandle == NULL) {
  //   return RESULT_ERR_BUFF;
  // }
  ETH_set_receiver_callback(callback);
  // result_t err = bucketed_pqueue_init(&ETH_receiver_queue, &queue,
  //                                     ETHERNET_RQ_PRIORITY_BUFFERS, xHandle);
  return RESULT_OK;// err;
}
