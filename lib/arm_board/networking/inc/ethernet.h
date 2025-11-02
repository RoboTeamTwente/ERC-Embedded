#ifndef ETHERNET_H
#define ETHERNET_H

#include "timer_lib"
#include <stdint.h>

#ifdef TEST
typedef void (*linkstatus_callback_t)(void *netif);
typedef void (*receiver_callback)(void *payload, size_t length);
#else
#include "ethernet_diagnostics.h"
#inlcude "ethernet_receiver.h"
#endif // !TEST

/**
 * @brief Initializes ethernet
 *
 * @param[in] htim1 Timer used for periodocaly network diagnostics
 *                  Timer has to be initialized before calling the function
 *
 * @param[in] receiver_callback callback function for receiving a message
 * @param[in] link_state_change_callback callback function for when the physical
 *                                       Ethernet link state changes
 *                                       I.e. cable connects/disconnects
 */
void ETH_init(TIM_HandleTypeDef *htim, receiver_callback receiver_callback,
              linkstatus_callback_t link_state_change_callback);

/**
 * @brief Initializes the udp stack, such that messages can be send.
 */
void ETH_udp_init();

/**
 * @brief Send a udp message
 *
 * @param ip[4]    Destination ip, 1 byte per entry
 * @param port     Destination port
 * @param payload  payload of the message
 */
void ETH_udp_send(uint8_t ip[4], uint8_t port, char *payload);

/**
 * @brief Send a raw ethernet frame
 *
 * @param mac[6] Destination mac address
 * @param payload payload of the message
 */
void ETH_raw_send(uint8_t mac[6], char *payload);

#endif
