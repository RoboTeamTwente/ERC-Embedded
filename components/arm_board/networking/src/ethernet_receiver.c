#include "ethernet_receiver.h"
#include "cmsis_os2.h"
#include "logging.h"
#include "pbuf.h"
#include "stm32h7xx_hal_eth.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "queue.h"
#define TAG "etherent_receiver"

extern ETH_HandleTypeDef heth;

receiver_callback r_callback;
QueueHandle_t receiveQueue;


const osThreadAttr_t receiver_callback_task_attr = {
    .name = "receiverCallbackTask",
    .stack_size = 1024 * 8,
    .priority = (osPriority_t)osPriorityNormal,
};
void ETH_receiver_callback_task(void *arg)
{
     receive_packet *buf;

    while (1) {
        // if (xQueueReceive(receiveQueue, &buf, portMAX_DELAY) == pdPASS) {
        //     r_callback(buf->data, buf->len);
        //     // Free buffer
        //     vPortFree(buf->data);
            
        // }
    }
}


void send_pbuf_to_queue(struct pbuf *p, QueueHandle_t queue)
{
    receive_packet packet;
    packet.data = pvPortMalloc(p->tot_len);  
    if (!packet.data) return;

    // Determine total length
    uint16_t total_len = p->tot_len;

    // Allocate buffer (static, or from pool)

    // Copy pbuf contents
    int16_t len = pbuf_copy_partial(p, packet.data, total_len, 0);
    packet.len = len;

    // Send buffer pointer to queue (ISR-safe)
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    xQueueSendFromISR(queue, &packet, &xHigherPriorityTaskWoken);
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
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
  free(data);
}

int8_t ETH_receiver_callback(struct pbuf *pbuf, struct netif *netif) {
  send_pbuf_to_queue(pbuf, receiveQueue);
  return tcpip_input(pbuf, netif);

}
void ETH_set_receiver_callback(ETH_HandleTypeDef *heth, struct netif *netif,
                               receiver_callback callback) {

  receiveQueue = xQueueCreate(10, sizeof(uint8_t *));
  r_callback = callback;
  netif->input = &ETH_receiver_callback;
 // osThreadNew(ETH_receiver_callback_task, NULL, &receiver_callback_task_attr);


}


