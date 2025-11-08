#include "ethernet_receiver.h"
#include "logging.h"
#include "pbuf.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define TAG "etherent_receiver"

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

void ETH_input_callback_example(void *payload, size_t length) {

  char *data = bytes_to_hex_string(payload, length);

  LOGI(TAG, "DATA RECEIVED: %s\n", data);
  free(data);
}

void ETH_input_callback(ETH_HandleTypeDef *heth, receiver_callback callback) {

  struct pbuf *buffer_pointer = NULL;
  if (HAL_ETH_ReadData(heth, (void **)&buffer_pointer) == HAL_OK) {

    callback(buffer_pointer->payload, heth->RxDescList.RxDataLength);

  } else {
    // TODO: return error
  }
  pbuf_free(
      buffer_pointer); // not confident about the stability of this, because
                       // normally it is called when netif.input != ERR_OK might
                       // only be needed when The buffer is full?
}
