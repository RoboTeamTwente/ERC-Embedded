
#include "ethernet_udp.h"
#include "FreeRTOS.h"
#include "bucketed_pqueue.h"
#include "err.h"
#include "ip4_addr.h"
#include "ip_addr.h"
#include "logging.h"
#include "networking_constants.h"
#include "pbuf.h"
#include "portmacro.h"
#include "projdefs.h"
#include "result.h"
#include "udp.h"
#include <lwip.h>
#include <stdint.h>
#include <string.h>
#define TAG "UDP"

extern ETH_HandleTypeDef heth;
receive_callback_t r_callback;
// receive queue variables
static StaticQueue_t xStaticQueue;
QueueHandle_t udp_receiver_queue;
uint8_t ucQueueStorageArea[ETHERNET_RQ_LENGTH * ETHERNET_RQ_ITEM_SIZE];
TaskHandle_t receiver_notifier = NULL;

#define STACK_SIZE 200
StaticTask_t xTaskBuffer;
StackType_t xStack[STACK_SIZE];

// send queue varialbes
bucketed_pqueue_t udp_send_bqueue;
TaskHandle_t send_notifier = NULL;

StaticTask_t sendTaskBuffer;
StackType_t sendStack[STACK_SIZE];

int counter = 0; // A counter for the send packet debug message

void udp_receiver_callback_example(receive_frame_t *rf) {

  uint8_t *bytes = (uint8_t *)(rf->payload);
  // Each byte becomes two hex characters, plus null terminator
  char *hex_str = malloc((rf->len) * 2 + 1);
  if (!hex_str)
    return;

  for (size_t i = 0; i < rf->len; ++i) {
    sprintf(&hex_str[i * 2], "%02X", bytes[i]);
  }

  LOGI(TAG, "DATA RECEIVED: %s\n", hex_str);
  LOGI(TAG, "Address: %d.%d.%d.%d - Port: %d", (uint8_t)(rf->addr.addr),
       (uint8_t)((rf->addr.addr) >> 8), (uint8_t)(rf->addr.addr >> 16),
       (uint8_t)(rf->addr.addr >> 24), rf->port);
  LOGI(TAG, "DMA ERROR CODE: %d\n", heth.DMAErrorCode);
  LOGI(TAG, "ERROR CODE: %d\n", heth.ErrorCode);
  free(hex_str);
}

/**
 * @brief UDP receiver function
 * @note When polling from  the used queue, free the payload.
 *
 * @param arg Arguments given to the receiver
 * @param pcb The current upd handler
 * @param p   The pbuf
 * @param addr The sender address
 * @param port The sender port
 */
void udp_receiver(void *arg, struct udp_pcb *pcb, struct pbuf *p,
                  const ip_addr_t *addr, u16_t port) {
  LOGI(TAG, "Port: %d", port);
  result_t err = RESULT_OK;
  receive_frame_t buffer = {
      .payload = malloc(p->len), .addr = *addr, .port = port, .len = p->len};
  if ((&buffer)->payload == NULL) {
    LOGE(TAG, "Couldn't allocate receive buffer");
    return;
  }
  memcpy(((&buffer)->payload), (int8_t *)(p->payload), p->len);
  if ((&buffer)->payload != NULL) {
    if (xQueueSend(udp_receiver_queue, &buffer, 10) != pdPASS) {
      err = RESULT_ERR_OVERFLOW;
    } else {
      (void)xTaskNotify(receiver_notifier, (1UL << (uint32_t)RQ_ETHERNET_PRIO),
                        eSetBits);
    }
  } else {
    LOGE(TAG, "Buffer copy failed");
  }
  if (err != RESULT_OK) {
    LOGE(TAG, "Could not push incomming message to queue: %s",
         result_to_short_str(err));
  }
  pbuf_free(p);
}

void udp_receiver_task(void *pvParameters) {

  configASSERT((uint32_t)pvParameters == 1UL);

  for (;;) {
    (void)ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
    for (;;) {

      LOGI(TAG, "UDP receiver notification received");
      receive_frame_t frame;
      BaseType_t r = xQueueReceive(udp_receiver_queue, &frame, 10);
      if (r != pdPASS) {
        free(frame.payload);
        break;
      }
      r_callback(&frame);
      free(frame.payload);
    }
  }
}

void udp_send_task(void *pvParameters) {

  configASSERT((uint32_t)pvParameters == 1UL);

  for (;;) {
    (void)ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
    for (;;) {
      LOGI(TAG, "UDP message being send %d", counter);
      counter += 1;
      send_frame_t frame;
      result_t r = bucketed_pqueue_pop(&udp_send_bqueue, &frame);

      if (r != RESULT_OK) {
        free(frame.payload);
        break;
      }

      struct pbuf *txBuf;

      int len = frame.payload_len;
      txBuf = pbuf_alloc(PBUF_TRANSPORT, len, PBUF_RAM);

      if (txBuf != NULL) {
        err_t err = pbuf_take(txBuf, frame.payload, len);
        if (err != ERR_OK) {
          LOGE(TAG, "buffer could not be filled: %s \n", lwip_strerr(err));
          pbuf_free(txBuf);
          free(frame.payload);
          break;
        }
      } else {
        LOGE(TAG, "cannot allocate a pbuffer");
        free(frame.payload);
        break;
      }
      free(frame.payload);

      err_t err = udp_sendto(frame.upcb, txBuf, &frame.addr, frame.port);
      if (err != ERR_OK) {
        LOGE(TAG, "Message could not be send: %s \n", lwip_strerr(err));
      }
      pbuf_free(txBuf);
    }
  }
}

result_t ETH_udp_receiver_init(struct udp_pcb *pcb,
                               receive_callback_t receiver_callback) {

  udp_receiver_queue =
      xQueueCreateStatic(ETHERNET_RQ_LENGTH, ETHERNET_RQ_ITEM_SIZE,
                         ucQueueStorageArea, &xStaticQueue);

  receiver_notifier = xTaskCreateStatic(

      udp_receiver_task, /* Function that implements the task. */

      "udp receiver task", /* Text name for the task. */

      STACK_SIZE, /* Number of indexes in the xStack array. */

      (void *)1, /* Parameter passed into the task. */

      tskIDLE_PRIORITY, /* Priority at which the task is created. */

      xStack, /* Array to use as the task's stack. */

      &xTaskBuffer); /* Variable to hold the task's data structure. */

  if (udp_receiver_queue == NULL || receiver_notifier == NULL) {
    return RESULT_ERR_BUFF;
  }
  r_callback = receiver_callback != NULL ? receiver_callback
                                         : udp_receiver_callback_example;

  udp_recv(pcb, udp_receiver, NULL);
  return RESULT_OK;
}

result_t ETH_udp_send_init(struct udp_pcb *pcb, uint8_t num_buckets,
                           QueueHandle_t *send_queues) {
  TaskHandle_t sendHandle = NULL;
  sendHandle = xTaskCreateStatic(

      udp_send_task, /* Function that implements the task. */

      "udp send task", /* Text name for the task. */

      STACK_SIZE, /* Number of indexes in the xStack array. */

      (void *)1, /* Parameter passed into the task. */

      tskIDLE_PRIORITY, /* Priority at which the task is created. */

      sendStack, /* Array to use as the task's stack. */

      &sendTaskBuffer); /* Variable to hold the task's data structure. */

  if (sendHandle == NULL) {
    return RESULT_ERR_BUFF;
  }

  result_t err = bucketed_pqueue_init(&udp_send_bqueue, send_queues,
                                      num_buckets, sendHandle);
  if (err != RESULT_OK) {
    LOGE(TAG, "pbuffer failed to initialize with error code: %s",
         result_to_short_str(err));
    return err;
  }
  return RESULT_OK;
}

result_t udp_client_init(struct udp_pcb **upcb, uint8_t prio_num,
                         QueueHandle_t *send_queues,
                         receive_callback_t receiver_callback) {

  *upcb = udp_new(); // TODO: return error if this is NULL
  if (upcb == NULL) {
    LOGE(TAG, "Cannot create new udp handler");
    return RESULT_FAIL;
  }

  err_t err = udp_bind(*upcb, IP_ADDR_ANY, 8);
  if (err != ERR_OK) {
    LOGE(TAG, "Cannot bind the udp: %s", lwip_strerr(err));
    return RESULT_FAIL;
  }
  ETH_udp_receiver_init(*upcb, receiver_callback);
  ETH_udp_send_init(*upcb, prio_num, send_queues);
  return RESULT_OK;
}

result_t udp_client_send(struct udp_pcb *upcb, uint8_t dest_ip[4],
                         uint16_t port, uint8_t *payload, uint16_t payload_len,
                         uint8_t prio_buf) {
  err_t err = ERR_OK;

  uint8_t *txBuf = malloc(payload_len);
  memcpy(txBuf, payload, payload_len);

  ip_addr_t destIPaddr;
  IP_ADDR4(&destIPaddr, dest_ip[0], dest_ip[1], dest_ip[2], dest_ip[3]);

  send_frame_t buffer = {.addr = destIPaddr,
                         .payload = txBuf,
                         .payload_len = payload_len,
                         .upcb = upcb,
                         .port = port};

  err = bucketed_pqueue_push(&udp_send_bqueue, prio_buf, &buffer, 10U);
  if (err != RESULT_OK) {
    LOGE(TAG, "Could not push send message to queue");
    free(txBuf);
    return err;
  }

  return RESULT_OK;
}
