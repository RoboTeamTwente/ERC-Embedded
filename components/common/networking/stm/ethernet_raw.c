
#include "ethernet_raw.h"
#include "err.h"
#include "ethernet_diagnostics.h"
#include "logging.h"
#include "netif.h"
#include "pbuf.h"
#include "result.h"
#include <stdint.h>
#include <string.h>
#define LWIP_HOOK_UNKNOWN_ETH_PROTOCOL(pbuf, netif) eth_reader(netif, pbuf)
#define TAG "stm32_raw_eth_packet"

// Safe no-op default callback
static void raw_receiver_callback_noop(void *payload, size_t length) {
  (void)payload;
  (void)length;
  // Silently ignore unknown protocols if no handler is registered
}

raw_receiver_callback r_callback = raw_receiver_callback_noop;
extern ETH_HandleTypeDef heth;

result_t raw_packet_send(struct netif *netif, ETH_HandleTypeDef *heth,
                         uint8_t mac_address[6], char *payload) {
  result_t err = RESULT_OK;
  err_t err_default;
  size_t payload_len = strlen(payload);
  size_t data_size = sizeof(ethernet_frame_t) + strlen(payload);

  ethernet_frame_t *frame = malloc(data_size);
  if (!frame) {
    err = RESULT_ERR_NO_MEM;
    LOGE("Could not send the message: %s \n", result_to_short_str(err));
    return err;
  }

  memcpy(frame->dest_mac, mac_address, 6);
  memcpy(frame->src_mac, heth->Init.MACAddr, 6);
  memcpy(frame->payload, payload, payload_len);

  struct pbuf *txBuf;

  txBuf = pbuf_alloc(PBUF_RAW, data_size, PBUF_RAM);

  if (txBuf != NULL) {
    err_default = pbuf_take(txBuf, frame, data_size);
    if (err_default != ERR_OK) {
      LOGE(TAG, "buffer could not be filled: %s \n", lwip_strerr(err));
      pbuf_free(txBuf);
      free(frame);
      return RESULT_ERR_BUFF;
    }
    if (netif_is_link_up(netif)) {
      err_default = netif->linkoutput(netif, txBuf);

      if (err_default != ERR_OK) {
        LOGE(TAG, "Could not send the message: %s", lwip_strerr(err_default));
        err = RESULT_FAIL;
      }
    } else {
      err = RESULT_ERR_COMMS;
      LOGE(TAG, "Connection is not up: %s \n", result_to_short_str(err));
    }
  } else {
    err = RESULT_ERR_BUFF;
    LOGE(TAG, "Could not send the message: %s \n", result_to_short_str(err));
    free(frame);
    return err;
  }

  pbuf_free(txBuf);

  free(frame);
  return err;
}

result_t raw_packet_send_binary(struct netif *netif, ETH_HandleTypeDef *heth,
                                uint8_t mac_address[6], void *payload, size_t length) {
  result_t err = RESULT_OK;
  err_t err_default;
  size_t data_size = sizeof(ethernet_frame_t) + length;

  ethernet_frame_t *frame = malloc(data_size);
  if (!frame) {
    err = RESULT_ERR_NO_MEM;
    LOGE(TAG, "Could not allocate memory for frame\n");
    return err;
  }

  memcpy(frame->dest_mac, mac_address, 6);
  memcpy(frame->src_mac, heth->Init.MACAddr, 6);
  frame->ethertype = htons(ETHERTYPE_SENSOR_DATA);  // Convert to network byte order
  memcpy(frame->payload, payload, length);

  struct pbuf *txBuf;

  txBuf = pbuf_alloc(PBUF_RAW, data_size, PBUF_RAM);

  if (txBuf != NULL) {
    err_default = pbuf_take(txBuf, frame, data_size);
    if (err_default != ERR_OK) {
      LOGE(TAG, "buffer could not be filled: %s \n", lwip_strerr(err_default));
      pbuf_free(txBuf);
      free(frame);
      return RESULT_ERR_BUFF;
    }
    if (netif_is_link_up(netif)) {
      err_default = netif->linkoutput(netif, txBuf);

      if (err_default != ERR_OK) {
        LOGE(TAG, "Could not send the message: %s", lwip_strerr(err_default));
        err = RESULT_FAIL;
      }
    } else {
      err = RESULT_ERR_COMMS;
      LOGE(TAG, "Connection is not up\n");
    }
  } else {
    err = RESULT_ERR_BUFF;
    LOGE(TAG, "Could not allocate pbuffer\n");
    free(frame);
    return err;
  }

  pbuf_free(txBuf);

  free(frame);
  return err;
}

u8_t eth_reader(struct netif *netif, struct pbuf *p) {
  r_callback(p->payload, p->len);
  return 1; // not handled, we never handle it, because I have no clue what I am
            // doing
}

void raw_receiver_callback_example(void *payload, size_t length) {

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

void raw_init(raw_receiver_callback callback) {
  if (callback != NULL) {
    r_callback = callback;
  } else {
    r_callback = raw_receiver_callback_example;
  }
}
