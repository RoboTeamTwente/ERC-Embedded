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
#include <stdlib.h>
#include <string.h>

#define TAG "UDP"

udp_receiver_callback r_callback;
extern ETH_HandleTypeDef heth;
receive_callback_t r_callback;

<<<<<<< HEAD
static struct udp_pcb *udp_sender_pcb;

bucketed_pqueue_t udp_receiver_queue;
bucketed_pqueue_t udp_sender_queue;
=======
int receive_counter = 0;
// receive queue variables
>>>>>>> 2aab4a75d8b73e7179c9866939b138f2fe84fea4
static StaticQueue_t xStaticQueue;
QueueHandle_t udp_queue;
uint8_t ucQueueStorageArea[ETHERNET_RQ_LENGTH * ETHERNET_RQ_ITEM_SIZE];

#define STACK_SIZE 200
StaticTask_t xTaskBuffer;
StackType_t xStack[STACK_SIZE];

<<<<<<< HEAD
#define TX_STACK_SIZE 256
StaticTask_t txTaskBuffer;
StackType_t txStack[TX_STACK_SIZE];
=======
// send queue varialbes
bucketed_pqueue_t udp_send_bqueue;
TaskHandle_t send_notifier = NULL;

StaticTask_t sendTaskBuffer;
StackType_t sendStack[STACK_SIZE];

int counter = 0; // A counter for the send packet debug message
>>>>>>> 2aab4a75d8b73e7179c9866939b138f2fe84fea4

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
<<<<<<< HEAD
  receive_frame buffer = { .payload = malloc(p->len), .addr = *addr, .port = port, .len = p->len };
=======
  receive_frame_t buffer = {
      .payload = malloc(p->len), .addr = *addr, .port = port, .len = p->len};
>>>>>>> 2aab4a75d8b73e7179c9866939b138f2fe84fea4
  if ((&buffer)->payload == NULL) {
    LOGE(TAG, "Couldn't allocate receive buffer");
    return;
  }
  memcpy(((&buffer)->payload), (int8_t *)(p->payload), p->len);
  if ((&buffer)->payload != NULL) {
    err = bucketed_pqueue_push(&udp_receiver_queue, 0, &buffer, 0U);
  } else {
    LOGE(TAG, "Buffer copy failed");
  }
  if (err != RESULT_OK) {
    LOGE(TAG, "Could not push incomming message to queue");
  }
  pbuf_free(p);
}

void udp_receiver_task(void *pvParameters) {

  configASSERT((uint32_t)pvParameters == 1UL);

  for (;;) {
    (void)ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
<<<<<<< HEAD
    for (;;) {
      receive_frame frame;
      result_t r = bucketed_pqueue_pop(&udp_receiver_queue, &frame);
      if (r != RESULT_OK) {
        break;
      }
      r_callback(frame.payload, frame.len, &(frame.addr), frame.port);
=======
    LOGI(TAG, "UDP receiver notification received");
    receive_frame_t frame;
    BaseType_t r = xQueueReceive(udp_receiver_queue, &frame, 10);
    if (r != pdPASS) {
>>>>>>> 2aab4a75d8b73e7179c9866939b138f2fe84fea4
      free(frame.payload);
      continue;
    }
    r_callback(&frame);
    receive_counter += 1;
    LOGI(TAG, "Received message: %d", receive_counter);
    free(frame.payload);
  }
}

void udp_sender_task(void *pvParameters) {

  configASSERT((uint32_t)pvParameters == 1UL);

  for (;;) {
    (void)ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
<<<<<<< HEAD
    for (;;) {
      udp_send_frame_t frame;
      result_t r = bucketed_pqueue_pop(&udp_sender_queue, &frame);
      if (r != RESULT_OK) {
        break;
      }
      if (udp_sender_pcb == NULL) {
        LOGE(TAG, "UDP sender not initialized");
        free(frame.payload);
        continue;
      }
      r = udp_client_send_binary(udp_sender_pcb, frame.dest_ip, frame.port,
                                 frame.payload, frame.len);
      if (r != RESULT_OK) {
        LOGE(TAG, "UDP send failed: %s", result_to_short_str(r));
      }
      free(frame.payload);
=======
    LOGI(TAG, "UDP message being send %d", counter);
    counter += 1;
    send_frame_t frame;
    result_t r = bucketed_pqueue_pop(&udp_send_bqueue, &frame);

    if (r != RESULT_OK) {
      free(frame.payload);
      LOGE(TAG, "Queue couldn't be popped");
      continue;
>>>>>>> 2aab4a75d8b73e7179c9866939b138f2fe84fea4
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
        continue;
      }
    } else {
      LOGE(TAG, "cannot allocate a pbuffer");
      free(frame.payload);
      continue;
    }
    free(frame.payload);

    err_t err = udp_sendto(frame.upcb, txBuf, &frame.addr, frame.port);
    if (err != ERR_OK) {
      LOGE(TAG, "Message could not be send: %s \n", lwip_strerr(err));
    }
    pbuf_free(txBuf);
  }
}

result_t ETH_udp_receiver_init(struct udp_pcb *pcb,
<<<<<<< HEAD
                               udp_receiver_callback udp_callback) {
=======
                               receive_callback_t receiver_callback) {
>>>>>>> 2aab4a75d8b73e7179c9866939b138f2fe84fea4

  udp_queue = xQueueCreateStatic(ETHERNET_RQ_LENGTH, ETHERNET_RQ_ITEM_SIZE,
                                 ucQueueStorageArea, &xStaticQueue);

  TaskHandle_t xHandle = NULL;
  xHandle = xTaskCreateStatic(

      udp_receiver_task, /* Function that implements the task. */

      "udp receiver task", /* Text name for the task. */

      STACK_SIZE, /* Number of indexes in the xStack array. */

      (void *)1, /* Parameter passed into the task. */

      tskIDLE_PRIORITY, /* Priority at which the task is created. */

      xStack, /* Array to use as the task's stack. */

      &xTaskBuffer); /* Variable to hold the task's data structure. */

  if (udp_queue == NULL || xHandle == NULL) {
    return RESULT_ERR_BUFF;
  }
  r_callback = receiver_callback != NULL ? receiver_callback
                                         : udp_receiver_callback_example;

  result_t err = bucketed_pqueue_init(&udp_receiver_queue, &udp_queue,
                                      ETHERNET_RQ_PRIORITY_BUFFERS, xHandle);
  if (err != RESULT_OK) {
    LOGE(TAG, "pbuffer failed to initialize with error code: %s",
         result_to_short_str(err));
  }
  if (udp_callback != NULL) {
    r_callback = udp_callback;
  } else {
    r_callback = udp_receiver_callback_example;
  }
  udp_recv(pcb, udp_receiver, NULL);
  return RESULT_OK;
}

result_t udp_client_init(struct udp_pcb **upcb, uint8_t src_ip[4],
                         udp_receiver_callback udp_callback) {

  if (src_ip == NULL) {
    LOGE(TAG, "Source IP is NULL");
    return RESULT_ERR_INVALID_ARG;
  }

  *upcb = udp_new(); // TODO: return error if this is NULL
  if (*upcb == NULL) {
    LOGE(TAG, "Cannot create new udp handler");
    return RESULT_FAIL;
  }
  ip_addr_t myIPaddr;
  IP_ADDR4(&myIPaddr, src_ip[0], src_ip[1], src_ip[2], src_ip[3]);
  err_t err = udp_bind(*upcb, &myIPaddr, 8);
  if (err != ERR_OK) {
    LOGE(TAG, "Cannot bind the udp: %s", lwip_strerr(err));
    return RESULT_FAIL;
  }
  udp_sender_pcb = *upcb;
  ETH_udp_receiver_init(*upcb, udp_callback);
  return RESULT_OK;
}

result_t udp_sender_init(uint8_t prio_num, QueueHandle_t *send_queues) {
  if (send_queues == NULL || prio_num == 0U) {
    return RESULT_ERR_INVALID_ARG;
  }

  TaskHandle_t xHandle = NULL;
  xHandle = xTaskCreateStatic(

      udp_sender_task, /* Function that implements the task. */

      "udp sender task", /* Text name for the task. */

      TX_STACK_SIZE, /* Number of indexes in the txStack array. */

      (void *)1, /* Parameter passed into the task. */

      tskIDLE_PRIORITY, /* Priority at which the task is created. */

      txStack, /* Array to use as the task's stack. */

      &txTaskBuffer); /* Variable to hold the task's data structure. */

  if (xHandle == NULL) {
    return RESULT_ERR_BUFF;
  }

  return bucketed_pqueue_init(&udp_sender_queue, send_queues, prio_num, xHandle);
}

<<<<<<< HEAD
result_t udp_client_send_enqueue(uint8_t dest_ip[4], uint8_t port,
                                 const void *payload, size_t length,
                                 uint8_t prio) {
  if (dest_ip == NULL || payload == NULL || length == 0U) {
    return RESULT_ERR_INVALID_ARG;
  }
  if (udp_sender_queue.buckets == NULL || udp_sender_queue.num_buckets == 0U) {
    return RESULT_ERR_NOT_INITIALIZED;
  }
  if (prio >= udp_sender_queue.num_buckets) {
    return RESULT_ERR_INVALID_ARG;
=======
result_t udp_client_init(struct udp_pcb **upcb, uint8_t prio_num,
                         QueueHandle_t *send_queues,
                         receive_callback_t receiver_callback) {

  *upcb = udp_new(); // TODO: return error if this is NULL
  if (upcb == NULL) {
    LOGE(TAG, "Cannot create new udp handler");
    return RESULT_FAIL;
>>>>>>> 2aab4a75d8b73e7179c9866939b138f2fe84fea4
  }

  void *payload_copy = malloc(length);
  if (payload_copy == NULL) {
    return RESULT_ERR_NO_MEM;
  }
<<<<<<< HEAD
  memcpy(payload_copy, payload, length);

  udp_send_frame_t frame = {
      .dest_ip = {dest_ip[0], dest_ip[1], dest_ip[2], dest_ip[3]},
      .port = port,
      .len = length,
      .payload = payload_copy,
  };

  result_t r = bucketed_pqueue_push(&udp_sender_queue, prio, &frame, 0U);
  if (r != RESULT_OK) {
    free(payload_copy);
  }
  return r;
=======
  ETH_udp_receiver_init(*upcb, receiver_callback);
  ETH_udp_send_init(*upcb, prio_num, send_queues);
  return RESULT_OK;
>>>>>>> 2aab4a75d8b73e7179c9866939b138f2fe84fea4
}

result_t udp_client_send(struct udp_pcb *upcb, uint8_t dest_ip[4], uint8_t port,
                         char *payload) {
  err_t err = ERR_OK;

  size_t payload_len = strlen(payload);
  struct pbuf *txBuf;
  char data[payload_len];

  int len = sprintf(data, "%s", payload);
  txBuf = pbuf_alloc(PBUF_TRANSPORT, len, PBUF_RAM);

<<<<<<< HEAD
  if (txBuf != NULL) {
    err = pbuf_take(txBuf, data, len);
    if (err != ERR_OK) {
      LOGE(TAG, "buffer could not be filled: %s \n", lwip_strerr(err));
      pbuf_free(txBuf);
      return RESULT_ERR_BUFF;
    }
=======
  send_frame_t buffer = {.addr = destIPaddr,
                         .payload = txBuf,
                         .payload_len = payload_len,
                         .upcb = upcb,
                         .port = port};
>>>>>>> 2aab4a75d8b73e7179c9866939b138f2fe84fea4

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

result_t udp_client_send_binary(struct udp_pcb *upcb, uint8_t dest_ip[4],
                                uint8_t port, void *payload, size_t length) {
  err_t err = ERR_OK;
  struct pbuf *txBuf;

  txBuf = pbuf_alloc(PBUF_TRANSPORT, length, PBUF_RAM);

  if (txBuf != NULL) {
    err = pbuf_take(txBuf, payload, length);
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
