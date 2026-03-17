
#include "ethernet_udp.h"
#include "bucketed_pqueue.h"
#include "ip4_addr.h"
#include "ip_addr.h"
#include "logging.h"
#include "networking_constants.h"
#include "result.h"
#include "udp.h"
#include <lwip.h>
#include <stdint.h>
#include <string.h>
#define TAG "UDP"

udp_receiver_callback r_callback;
extern ETH_HandleTypeDef heth;

static StaticQueue_t xStaticQueue;
QueueHandle_t udp_receiver_queue;
uint8_t ucQueueStorageArea[ETHERNET_RQ_LENGTH * ETHERNET_RQ_ITEM_SIZE];
TaskHandle_t receiver_notifier = NULL;

#define STACK_SIZE 200
StaticTask_t xTaskBuffer;
StackType_t xStack[STACK_SIZE];

void udp_receiver_callback_example(void *payload, size_t length,
                                   const ip_addr_t *addr, u16_t port) {

  uint8_t *bytes = (uint8_t *)payload;
  // Each byte becomes two hex characters, plus null terminator
  char *hex_str = malloc(length * 2 + 1);
  if (!hex_str)
    return;

  for (size_t i = 0; i < length; ++i) {
    sprintf(&hex_str[i * 2], "%02X", bytes[i]);
  }

  LOGI(TAG, "DATA RECEIVED: %s\n", hex_str);
  LOGI(TAG, "Address: %d.%d.%d.%d - Port: %d", (uint8_t)(addr->addr),
       (uint8_t)((addr->addr) >> 8), (uint8_t)(addr->addr >> 16),
       (uint8_t)(addr->addr >> 24), port);
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
  receive_frame buffer = {
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
      (void)xTaskNotify(receiver_notifier, (1UL << (uint32_t)ETHERNET_PRIO),
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
      receive_frame frame;
      result_t r = xQueueReceive(udp_receiver_queue, &frame, 10);
      if (r != RESULT_OK) {
        free(frame.payload);
        break;
      }
      r_callback(frame.payload, frame.len, &(frame.addr), frame.port);
      free(frame.payload);
    }
  }
}

result_t ETH_udp_receiver_init(struct udp_pcb *pcb,
                               udp_receiver_callback udp_callback) {

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

  if (udp_callback != NULL) {
    r_callback = udp_callback;
  } else {
    r_callback = udp_receiver_callback_example;
  }
  udp_recv(pcb, udp_receiver, NULL);

  return RESULT_OK;
}

result_t udp_client_init(struct udp_pcb **upcb,
                         udp_receiver_callback udp_callback) {

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
  ETH_udp_receiver_init(*upcb, udp_callback);
  return RESULT_OK;
}

result_t udp_client_send(struct udp_pcb *upcb, uint8_t dest_ip[4], uint8_t port,
                         char *payload) {
  err_t err = ERR_OK;

  size_t payload_len = strlen(payload);
  struct pbuf *txBuf;
  char data[payload_len];

  int len = sprintf(data, "%s", payload);
  txBuf = pbuf_alloc(PBUF_TRANSPORT, len, PBUF_RAM);

  if (txBuf != NULL) {
    err = pbuf_take(txBuf, data, len);
    if (err != ERR_OK) {
      LOGE(TAG, "buffer could not be filled: %s \n", lwip_strerr(err));
      pbuf_free(txBuf);
      return RESULT_ERR_BUFF;
    }

    ip_addr_t destIPaddr;
    IP_ADDR4(&destIPaddr, dest_ip[0], dest_ip[1], dest_ip[2], dest_ip[3]);
    err = udp_sendto(upcb, txBuf, &destIPaddr, port);
    if (err != ERR_OK) {
      LOGE(TAG, "Message could not be send: %s \n", lwip_strerr(err));
      pbuf_free(txBuf);
      return RESULT_ERR_COMMS;
    }
    pbuf_free(txBuf);
  } else {
    LOGE(TAG, "cannot allocate a pbuffer");
    return RESULT_ERR_BUFF;
  }

  return RESULT_OK;
}
