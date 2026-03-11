#ifndef ETHERNET_H
#define ETHERNET_H

#include <stdint.h>

#include "result.h"
#include "stm/ethernet_diagnostics.h"
#include "stm/ethernet_raw.h"

/**
 * @brief Initializes ethernet
 *
 *
 * @param[in] receiver_callback callback function for receiving a message
 * @param[in] link_state_change_callback callback function for when the physical
 *                                       Ethernet link state changes
 *                                       I.e. cable connects/disconnects
 */
void ETH_init(receiver_callback receiver_callback,
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
result_t ETH_add_arp(int ip[4], int mac[6]);

/**
 * @brief Send a raw ethernet frame
 *
 * @param mac[6] Destination mac address
 * @param payload payload of the message
 */
void ETH_raw_send(uint8_t mac[6], char *payload);

/**
 * @brief initilaizes raw ethernet
 *
 * @param callback receiver callback function
 */
void ETH_raw_init(raw_receiver_callback callback);
#endif
