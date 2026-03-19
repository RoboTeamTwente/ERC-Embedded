#ifndef ETHERNET_UDP
#define ETHERNET_UDP

#include "err.h"
#include "ip4_addr.h"
#include "result.h"
#include "udp.h"
#include <stdint.h>

typedef struct {
  ip_addr_t addr;
  void *payload;
  u16_t port;
  uint16_t len;

} receive_frame;

/**
 * @brief sends a udp packet, it tries to resolve the mac adress first
 *
 * @param[in] upcb pointer to a udp handler
 * @param[in] dest_ip pointer to an int list of 4 ints that make up the
 * destination IP
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
 * @return result_t
 */
result_t udp_client_init(struct udp_pcb **upcb);

#endif // !ETHERNET_UDP
