
#include "ethernet_udp_sender.h"
#include "logging.h"
#include "udp.h"
#include <lwip.h>
#include <stdint.h>
#include <string.h>
#define TAG "UDP"

/**
 * @brief sends a udp packet, it tries to resolve the mac adress first
 *
 * @param[in] upcb pointer to a udp handler
 * @param[in] dest_ip pointer to an int list of 4 ints that make up the
 * destination IP
 * @param[in] length dest_ip length. Should be always 4
 * @param[in] port destination port address
 * @param[in] payload String with the payload
 * @return err_t
 */
err_t udp_client_send(struct udp_pcb *upcb, uint8_t dest_ip[4], uint8_t port,
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
      LOGE(TAG, "buffer could not be filled: %d \n", err);
    }

    ip_addr_t destIPaddr;
    IP_ADDR4(&destIPaddr, dest_ip[0], dest_ip[1], dest_ip[2], dest_ip[3]);
    err = udp_sendto(upcb, txBuf, &destIPaddr, port);
    if (err != ERR_OK) {
      LOGE(TAG, "Message could not be send: %d \n", err);
    }
    pbuf_free(txBuf);
  }
  return err; // TODO: make this return when the error is found, not at the end
}

/**
 * @brief initializes a udp_handler
 *
 * @param[out] upcb pointer to a UDP handler
 * @param[in] src_ip pointer to an array with the source ip of length 4
 * @param[in] len lenght of the ip. Should always be 4
 * @return err_t
 */
err_t udp_client_init(struct udp_pcb **upcb, uint8_t src_ip[4]) {

  *upcb = udp_new(); // TODO: return error if this is NULL
  ip_addr_t myIPaddr;
  IP_ADDR4(&myIPaddr, 192, 168, 0, 111);
  err_t err = udp_bind(*upcb, &myIPaddr, 8);
  // TODO: Log error
  return err;
}
