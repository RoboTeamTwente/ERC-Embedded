#ifndef ETHERNET_H
#define ETHERNET_H

#include <stddef.h>
#include <stdint.h>

#include "FreeRTOS.h"
#include "queue.h"
#include "result.h"
#include "stm/ethernet_diagnostics.h"
#include "stm/ethernet_raw.h"
#include "stm/ethernet_udp.h"
/**
 * @brief Initializes ethernet
 *
 * @param[in] link_state_change_callback callback function for when the physical
 *                                       Ethernet link state changes
 *                                       I.e. cable connects/disconnects
 */
void ETH_init(linkstatus_callback_t link_state_change_callback);

/**
 * @brief Initializes the udp stack with a priority send queue.
 *
 * @param[in] sender_prio_num number of priority buckets
 * @param[in] send_queues array of queues, length sender_prio_num
 * @param[in] callback UDP receive callback (NULL uses default logger)
 * @return result_t
 */
result_t ETH_udp_init(uint8_t sender_prio_num, QueueHandle_t *send_queues,
                      udp_receiver_callback callback);

/**
 * @brief Send a udp message using the priority send queue
 *
 * @param ip[4]    Destination ip, 1 byte per entry
 * @param port     Destination port
 * @param payload  payload of the message
 * @param prio     priority bucket
 * @return result_t
 */
result_t ETH_udp_send(uint8_t ip[4], uint8_t port, const char *payload,
					  uint8_t prio);

/**
 * @brief Send a udp message with binary data using the priority send queue
 *
 * @param ip[4]    Destination ip, 1 byte per entry
 * @param port     Destination port
 * @param payload  pointer to binary data
 * @param length   length of payload in bytes
 * @param prio     priority bucket
 * @return result_t
 */
result_t ETH_udp_send_binary(uint8_t ip[4], uint8_t port, const void *payload,
							 size_t length, uint8_t prio);

/**
 * @brief Sets registers for perfect mac filtering
 *
 * @param[in] mac1 mac to filter, can be NULL
 * @param[in] mac2 mac to filter, can be NULL
 * @param[in] mac3 mac to filter, can be NULL
 */
void ETH_setup_MAC_address_filtering(int mac1[6], int mac2[6], int mac3[6]);

/**
 * @brief Add an IP to the arp table
 *
 * @param ip  the ip
 * @param mac the mac
 * @return RESULT_OK if the element is succesfully added,
 *         RESULT_ERR_COMMS if it is not succesfully added.
 */
result_t ETH_add_arp(uint8_t ip[4], uint8_t mac[6]);

/**
 * @brief Send a raw ethernet frame
 *
 * @param mac[6] Destination mac address
 * @param payload payload of the message
 */
void ETH_raw_send(uint8_t mac[6], char *payload);

/**
 * @brief Send a raw ethernet frame with binary data
 *
 * @param mac[6] Destination mac address
 * @param payload pointer to binary data
 * @param length length of payload in bytes
 */
void ETH_raw_send_binary(uint8_t mac[6], void *payload, size_t length);

/**
 * @brief initilaizes raw ethernet
 *
 * @param callback receiver callback function
 */
void ETH_raw_init(raw_receiver_callback callback);
#endif
