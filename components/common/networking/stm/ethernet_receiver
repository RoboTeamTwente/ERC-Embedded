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
#include "udp.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define TAG "etherent_receiver"
#define LWIP_HOOK_UNKNOWN_ETH_PROTOCOL(pbuf, netif) eth_reader(netif, pbuf)

extern ETH_HandleTypeDef heth;

receiver_callback r_callback;

u8_t eth_reader(struct netif *netif, struct pbuf *p) {
  r_callback(p->payload, p->len);
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

void udp_receiver(void *arg, struct udp_pcb *pcb, struct pbuf *p,
                  const ip_addr_t *addr, u16_t port) {
  r_callback(p->payload, p->len);
  pbuf_free(p);

}

result_t ETH_udp_receiver_init(struct udp_pcb *pcb) {
  udp_recv(pcb, udp_receiver, NULL);
  return RESULT_OK;
}

result_t ETH_receiver_init(receiver_callback callback) {

  ETH_set_receiver_callback(callback);

  return RESULT_OK; // err;
}
