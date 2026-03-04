#ifndef ETHERNET_UDP
#define ETHERNET_UDP

#include "err.h"
#include "ip4_addr.h"
#include "result.h"
#include "udp.h"
#include <stdint.h>



typedef struct {
    void* payload;
    uint16_t len;
} receive_frame;

/**
 * @brief The typedef of the callback function for receiving a message
 *
 * @param payload A pointer to the payload
 * @param length The lenght of the payload
 * @param[in] addr The source ip
 * @param[in] port The source port
 */
typedef void (*udp_receiver_callback)(void *payload, size_t length,
                                      const ip_addr_t *addr, u16_t port);

/**
 * @brief sends a udp packet, it tries to resolve the mac adress first
 *
 * @param[in] upcb pointer to a udp handler
 * @param[in] dest_ip pointer to an int list of 4 ints that make up the
 * destination IP
 * @param[in] length dest_ip length. Should be always 4
 * @param[in] port destination port address
 * @param[in] payload String with the payload
 * @return err_t
 */
result_t udp_client_send(struct udp_pcb *upcb, uint8_t dest_ip[4], uint8_t port,
                         char *payload);

/**
 * @brief initializes a udp_handler. You can only initialize 1 callback
 * function, if you do it multiple times the last one is used.
 *
 * @param[out] upcb pointer to a UDP handler
 * @param[in] src_ip pointer to an array with the source ip of length 4
 * @param[in] len lenght of the ip. Should always be 4
 * @param[in] udp_callback the udp callback function
 * @return result_t
 */
result_t udp_client_init(struct udp_pcb **upcb, uint8_t src_ip[4],
                         udp_receiver_callback udp_callback);

#endif // !ETHERNET_UDP
