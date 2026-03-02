
#include "ethernet_udp.h"
#include "ip4_addr.h"
#include "ip_addr.h"
#include "logging.h"
#include "result.h"
#include "udp.h"
#include <lwip.h>
#include <stdint.h>
#include <string.h>
#define TAG "UDP"

udp_receiver_callback r_callback;
extern ETH_HandleTypeDef heth;

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

/**
 * @brief UDP receiver initializer
 *
 * @param[in] pcb The udp pcb
 * @return RESULT_OK if it initialized correctly
 */
void udp_receiver(void *arg, struct udp_pcb *pcb, struct pbuf *p,
                  const ip_addr_t *addr, u16_t port) {
  r_callback(p->payload, p->len, addr, port);
  pbuf_free(p);
}

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
  LOGI(TAG, "DMA ERROR CODE: %d\n", heth.DMAErrorCode);
  LOGI(TAG, "ERROR CODE: %d\n", heth.ErrorCode);
  free(hex_str);
}

result_t ETH_udp_receiver_init(struct udp_pcb *pcb,
                               udp_receiver_callback udp_callback) {
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

  *upcb = udp_new(); // TODO: return error if this is NULL
  if (upcb == NULL) {
    LOGE(TAG, "Cannot create new udp handler");
    return RESULT_FAIL;
  }
  ip_addr_t myIPaddr;
  IP_ADDR4(&myIPaddr, 192, 168, 0, 111);
  err_t err = udp_bind(*upcb, &myIPaddr, 8);
  if (err != ERR_OK) {
    LOGE(TAG, "Cannot bind the udp: %s", lwip_strerr(err));
    return RESULT_FAIL;
  }
  ETH_udp_receiver_init(*upcb, udp_callback);
  return RESULT_OK;
}
