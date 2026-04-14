/**
 * @file
 * @brief The header to be used by any ehternet implementation
 */

#ifndef ETHERNET_H
#define ETHERNET_H

#include "FreeRTOS.h"
#include "queue.h"
#include "result.h"
#include "stm/ethernet_diagnostics.h"
#include "stm/ethernet_raw.h"
#include "stm/ethernet_udp.h"
#include <stdint.h>
#include <sys/types.h>
/**
 * @brief Initializes ethernet
 *
 * @param[in] link_state_change_callback callback function for when the physical
 *                                       Ethernet link state changes
 *                                       I.e. cable connects/disconnects
 * @param ip The ip of the board
 * @param netmask The netmask of the board
 * @param gateway The gateway of the board
 * @param mac_address The mac_address of the board
 * @return An ethernet error
 */
HAL_StatusTypeDef ETH_init(linkstatus_callback_t link_state_change_callback,
                           uint8_t ip[4], uint8_t netmask[4],
                           uint8_t gateway[4], uint8_t mac_address[6]);

/**
 * @brief Initialize the UDP protocol control block.
 *
 * Configures the UDP subsystem, including priority-based transmit queues
 * and the receive callback handler.
 *
 * @param[in] sender_prio_num
 *      Number of priority levels for outgoing messages.
 *
 * @param[in] send_queues
 *      Array of queue handles used for prioritized message transmission.
 *      The array must contain at least @p sender_prio_num elements.
 *
 * @param[in] receiver_callback
 *      Callback function invoked when a UDP packet is received.
 */
void ETH_udp_init(uint8_t sender_prio_num, QueueHandle_t *send_queues,
                  receive_callback_t receiver_callback);

/**
 * @brief Send a UDP message.
 *
 * Queues a UDP packet for transmission to the specified destination using
 * the given priority level.
 *
 * @param[in] ip
 *      Destination IPv4 address (array of 4 bytes).
 *
 * @param[in] port
 *      Destination port number.
 *
 * @param[in] payload
 *      Pointer to the payload data buffer.
 *
 * @param[in] payload_len
 *      Length of the payload in bytes.
 *
 * @param[in] prio_num
 *      Priority level used for transmission. Must be less than the value
 *      specified in ETH_udp_init().
 */
void ETH_udp_send(uint8_t ip[4], uint8_t port, uint8_t *payload,
                  uint16_t payload_len, uint8_t prio_num);

/**
 * @brief Configure MAC address filtering.
 *
 * Sets up to three MAC addresses for perfect filtering. Only frames with a
 * destination addres matching one of the configured addresses, or the mac
 * address configured in ETH_init, will be accepted.
 *
 * @param[in] mac1
 *      Pointer to first MAC address (6 bytes). Can be NULL to disable.
 *
 * @param[in] mac2
 *      Pointer to second MAC address (6 bytes). Can be NULL to disable.
 *
 * @param[in] mac3
 *      Pointer to third MAC address (6 bytes). Can be NULL to disable.
 */
void ETH_setup_MAC_address_filtering(int mac1[6], int mac2[6], int mac3[6]);

/**
 * @brief Add an entry to the ARP table.
 *
 * Associates an IPv4 address with a MAC address for future lookups.
 *
 * @param[in] ip
 *      IPv4 address (array of 4 integers).
 *
 * @param[in] mac
 *      MAC address (array of 6 integers).
 *
 * @return RESULT_OK
 *      Entry successfully added.
 *
 * @return RESULT_ERR_COMMS
 *      Failed to add the entry.
 */

result_t ETH_add_arp(int ip[4], int mac[6]);

/**
 * @brief Send a raw Ethernet frame.
 *
 * Transmits a raw Ethernet payload directly to the specified MAC address.
 * No protocol headers (e.g., IP/UDP) are added automatically.
 *
 * @param[in] mac
 *      Destination MAC address (6 bytes).
 *
 * @param[in] payload
 *      Pointer to raw payload buffer.
 */
void ETH_raw_send(uint8_t mac[6], char *payload);

/**
 * @brief Register custom protocol callback.
 *
 * registers a callback for incoming frames with custom protocols.
 *
 * @param[in] callback
 *      Function invoked when a frame with an unkown protocol is received.
 */
void ETH_custom_protocol_receiver(raw_receiver_callback callback);
#endif
