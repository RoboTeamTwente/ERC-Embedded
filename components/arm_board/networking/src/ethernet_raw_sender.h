#ifndef ETHERNET_RAW_SENDER_H
#define ETHERNET_RAW_SENDER_H

#include "err.h"
#include "netif.h"
#include <stdint.h>
#include "result.h"
/**
 *
 * @brief sturct defining an raw ethernet frame.
 */
typedef struct {

  uint8_t dest_mac[6];
  uint8_t src_mac[6];
  uint8_t payload[];
} ethernet_frame_t;

/**
 * @brief Sends a raw ethernet packet
 *
 * @param netif Network interface
 * @param heth Ethernet handler
 * @param mac_address[6] destination mac address
 * @param payload payload
 *
 * @return error
 */
result_t raw_packet_send(struct netif *netif, ETH_HandleTypeDef *heth,
                      uint8_t mac_address[6], char *payload);

#endif // !ETHERNET_RAW_SENDER_H
