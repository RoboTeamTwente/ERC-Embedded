#include "ethernet.h"
#include "logging.h"
#include <stddef.h>

#define TAG "MAIN"

void ETH_udp_init() {}

void receiver_callback_default(const void *payload, size_t length) {

  printf("%.*s\n", (int)length, (const char *)payload);
}

void ETH_udp_send(uint8_t ip[4], uint8_t port, char *payload) {
  receiver_callback_default(payload, sizeof(payload));
}

void ETH_raw_send(uint8_t *mac, char *payload) {
  receiver_callback_default(payload, sizeof(payload));
}

void ETH_init(
    TIM_HandleTypeDef *htim, receiver_callback receiver_callback,
    linkstatus_callback_t link_state_change_callback) { // TODO: return an error
  LOG(TAG, "Setting up ethernet...\n");
  LOG(TAG, "Ethernet is set up!\n");
}
