#ifndef ETHERNET_RECEIVER_H
#define ETHERNET_RECEIVER_H

#include "result.h"
#include "udp.h"
#include <stddef.h>
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
void ETH_receiver_callback_example(void *payload, size_t length);

/**
 * @brief UDP receiver initializer
 *
 * @param[in] pcb The udp pcb
 * @return RESULT_OK if it initialized correctly
 */
result_t ETH_udp_receiver_init(struct udp_pcb *pcb);

/**
 * @brief Ethernet receiver initializer
 *
 * @param[in] callback The callback function, if NULL, a
 * ETH_receiver_callback_example is used
 *
 * @return RESULT_OK if it initialized correctly
 */
result_t ETH_receiver_init(receiver_callback callback);

#endif // !ETHERNET_RECEIVER_H
