
#include "ethernet_raw_sender.h"
#include "err.h"
#include "ethernet_diagnostics.h"
#include "ethernet_receiver.h"
#include "logging.h"
#include "netif.h"
#include "pbuf.h"
#include <stdint.h>
#include <string.h>

#define TAG "stm32_eth_packet"

err_t raw_packet_send(struct netif *netif, ETH_HandleTypeDef *heth,
                      uint8_t mac_address[6], char *payload) {
  err_t err = ERR_OK;
  size_t payload_len = strlen(payload);
  size_t data_size = sizeof(ethernet_frame_t) + strlen(payload);

  ethernet_frame_t *frame = malloc(data_size);
  if (!frame)
    return ERR_MEM;

  memcpy(frame->dest_mac, mac_address, 6);
  memcpy(frame->src_mac, heth->Init.MACAddr, 6);
  memcpy(frame->payload, payload, payload_len);

  struct pbuf *txBuf;

  txBuf = pbuf_alloc(PBUF_RAW, data_size, PBUF_RAM);

  if (txBuf != NULL) {
    memcpy(txBuf->payload, frame, data_size);
    if (err != ERR_OK) {
      LOGE(TAG, "buffer could not be filled: %d \n", err);
    }
    err = netif->linkoutput(netif, txBuf);
    if (err != ERR_OK) {
      LOGE(TAG, "Message could not be send: %d \n", err);
    }
    pbuf_free(txBuf);
  }
  free(frame);
  return err; // TODO: make this return when the error is found, not at the end
}
