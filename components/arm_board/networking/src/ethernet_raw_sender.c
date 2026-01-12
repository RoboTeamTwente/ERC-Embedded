
#include "ethernet_raw_sender.h"
#include "err.h"
#include "ethernet_diagnostics.h"
#include "ethernet_receiver.h"
#include "logging.h"
#include "netif.h"
#include "pbuf.h"
#include <stdint.h>
#include <string.h>
#include "result.h"

#define TAG "stm32_eth_packet"


result_t raw_packet_send(struct netif *netif, ETH_HandleTypeDef *heth,
                      uint8_t mac_address[6], char *payload) {
  result_t err = RESULT_OK;
  size_t payload_len = strlen(payload);
  size_t data_size = sizeof(ethernet_frame_t) + strlen(payload);

  ethernet_frame_t *frame = malloc(data_size);
  if (!frame){
    err = RESULT_ERR_NO_MEM;
    char* result = result_to_short_str(err);
    LOGE("Could not send the message: %s \n", result_to_short_str(err));
    return err;
  }

  memcpy(frame->dest_mac, mac_address, 6);
  memcpy(frame->src_mac, heth->Init.MACAddr, 6);
  memcpy(frame->payload, payload, payload_len);

  struct pbuf *txBuf;

  txBuf = pbuf_alloc(PBUF_RAW, data_size, PBUF_RAM);

  if (txBuf != NULL) {
    memcpy(txBuf->payload, frame, data_size);

    if (netif_is_link_up(netif)) {
      err_t err_default = netif->linkoutput(netif, txBuf);
      if(err_default != ERR_OK){
        LOGE(TAG, "Could not send the message: RESULT_FAIL");
        free(frame);
        return RESULT_FAIL;
      }
    }
    else{
      err = RESULT_ERR_COMMS;
      LOGE(TAG, "Connection is not up: %s \n", result_to_short_str(err));
      free(frame);
      return err;
    }
    pbuf_free(txBuf);
  }
  else{
    err = RESULT_ERR_BUFF;
    LOGE(TAG, "Could not send the message: %s \n", result_to_short_str(err));
  }
  free(frame);
  return err;
}
