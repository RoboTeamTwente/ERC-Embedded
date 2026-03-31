#ifndef ETHERNET_UDP
#define ETHERNET_UDP

#include "err.h"
#include "ip4_addr.h"
#include "result.h"
#include "udp.h"
#include <stdint.h>
#include <stddef.h>

#include "FreeRTOS.h"
#include "queue.h"

typedef struct {
  ip_addr_t addr;
  void *payload;
  u16_t port;
  uint16_t len;

} receive_frame;

typedef struct {
  uint8_t dest_ip[4];
  uint8_t port;
  size_t len;
  void *payload;
} udp_send_frame_t;

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
 * @brief sends a udp packet with binary data
 *
 * @param[in] upcb pointer to a udp handler
 * @param[in] dest_ip pointer to an int list of 4 ints that make up the
 * destination IP
 * @param[in] port destination port address
 * @param[in] payload pointer to binary data
 * @param[in] length length of payload in bytes
 * @return result_t
 */
result_t udp_client_send_binary(struct udp_pcb *upcb, uint8_t dest_ip[4], 
                                uint8_t port, void *payload, size_t length);

/**
 * @brief initialize UDP sender queue and task
 *
 * @param[in] prio_num number of priority buckets
 * @param[in] send_queues array of queue handles, length prio_num
 * @return result_t
 */
result_t udp_sender_init(uint8_t prio_num, QueueHandle_t *send_queues);

/**
 * @brief enqueue a UDP send request
 *
 * @param[in] dest_ip destination ip
 * @param[in] port destination port
 * @param[in] payload payload pointer
 * @param[in] length payload length
 * @param[in] prio priority bucket
 * @return result_t
 */
result_t udp_client_send_enqueue(uint8_t dest_ip[4], uint8_t port,
                                 const void *payload, size_t length,
                                 uint8_t prio);

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
