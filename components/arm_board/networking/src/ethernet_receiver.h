#ifndef ETHERNET_RECEIVER_H
#define ETHERNET_RECEIVER_H

#include "ethernet.h"
#include "lwip.h"

/**
 * @brief The typedef of the callback function for receiving a message
 *
 * @param payload A pointer to the payload
 * @param length The lenght of the payload
 */
typedef void (*receiver_callback)(void *payload, size_t length);
typedef struct {
    uint8_t *data;   // pointer to buffer
    uint16_t len;    // length of the buffer
} receive_packet;
/**
 * @brief Example function for the ethernet callback
 *
 * @param payload payload of the ethernet packet
 * @param length size of the payload
 */
void ETH_receiver_callback_example(void *payload, size_t length);

/**
 * @brief Ethernet receiver callback setter
 *
 * @param[in] heth Ethenet handler
 * @param[in] netif network interface
 * @param[in] callback The callback function
 */
void ETH_set_receiver_callback(ETH_HandleTypeDef *heth, struct netif *netif,
                               receiver_callback callback);




#endif // !ETHERNET_RECEIVER_H
