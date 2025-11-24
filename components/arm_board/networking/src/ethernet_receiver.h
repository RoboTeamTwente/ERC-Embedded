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

/**
 * @brief Example function for the ethernet callback
 *
 * @param payload payload of the ethernet packet
 * @param length size of the payload
 */
void ETH_input_callback_example(void *payload, size_t length);

/**
 * @brief Ethernet input callback function
 *
 * @param heth Ethenet handler
 * @param callback The callback function
 */
void ETH_input_callback(ETH_HandleTypeDef *heth, receiver_callback callback);

#endif // !ETHERNET_RECEIVER_H
