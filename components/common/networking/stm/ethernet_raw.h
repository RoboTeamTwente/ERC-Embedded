#ifndef ETHERNET_RAW_H
#define ETHERNET_RAW_H

#include "err.h"
#include "netif.h"
#include "result.h"
#include <stdint.h>

// Custom EtherType for sensor data packets (0xEEEE = experimental)
#define ETHERTYPE_SENSOR_DATA 0xEEEE

/**
 * @brief The typedef of the callback function for receiving a message
 *
 * @param payload A pointer to the payload
 * @param length The lenght of the payload
 */
typedef void (*raw_receiver_callback)(void *payload, size_t length);

/**
 *
 * @brief sturct defining an raw ethernet frame.
 */
typedef struct {
  uint8_t dest_mac[6];
  uint8_t src_mac[6];
  uint16_t ethertype;  // Network byte order (big-endian)
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

/**
 * @brief Sends a raw ethernet packet with binary data
 *
 * @param netif Network interface
 * @param heth Ethernet handler
 * @param mac_address[6] destination mac address
 * @param payload pointer to binary data
 * @param length length of payload in bytes
 *
 * @return error
 */
result_t raw_packet_send_binary(struct netif *netif, ETH_HandleTypeDef *heth,
                                uint8_t mac_address[6], void *payload, size_t length);

/**
 * @brief initializes the raw receiver
 *
 * @param callback the callback function for the receiver
 */
void raw_init(raw_receiver_callback callback);

#endif // !ETHERNET_RAW_H
