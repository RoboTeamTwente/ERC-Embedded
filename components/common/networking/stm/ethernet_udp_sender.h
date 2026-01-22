#ifndef ETHERNET_UDP_SENDER
#define ETHERNET_UDP_SENDER

#include "err.h"
#include "result.h"
#include "udp.h"
#include <stdint.h>

/**
 * @brief Sends a message over udp
 *
 * @param upcb the udp handler
 * @param dest_ip[4] the destination ip, 1 byte per entry
 * @param port the destination port
 * @param payload the payload
 *
 * @return error
 */
result_t udp_client_send(struct udp_pcb *upcb, uint8_t dest_ip[4], uint8_t port,
                         char *payload);

/**
 * @brief Initializes the udp client
 *
 * @param upcb the udp handler
 * @param src_ip[4] The source soure ip
 *
 * @return error
 */
result_t udp_client_init(struct udp_pcb **upcb, uint8_t src_ip[4]);

#endif // !ETHERNET_UDP_SENDER
