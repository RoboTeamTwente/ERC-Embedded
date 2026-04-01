#ifndef ETHERNET_UDP
#define ETHERNET_UDP

#include "FreeRTOS.h"
#include "err.h"
#include "ip4_addr.h"
#include "queue.h"
#include "result.h"
#include "udp.h"
#include <stdint.h>

typedef struct {
  ip_addr_t addr;
  void *payload;
  u16_t port;
  uint16_t len;

} receive_frame_t;

typedef struct {
  ip_addr_t addr;
  uint16_t port;
  uint16_t payload_len;
  uint8_t *payload;
  struct udp_pcb *upcb;

} send_frame_t;

typedef void (*receive_callback_t)(receive_frame_t *receive_frame);

/**
 * @brief sends a udp packet, it tries to resolve the mac adress first
 *
 * @param[in] upcb pointer to a udp handler
 * @param[in] dest_ip pointer to an int list of 4 ints that make up the
 * destination IP
 * @param[in] port destination port address
 * @param[in] payload String with the payload
 * @param[in] payload_len length of the payload
 * @param[in] priority of the packet
 * @return err_t
 */
result_t udp_client_send(struct udp_pcb *upcb, uint8_t dest_ip[4],
                         uint16_t port, uint8_t *payload, uint16_t payload_len,
                         uint8_t prio_buf);

/**
 * @brief initializes a udp_handler. You can only initialize 1 callback
 * function, if you do it multiple times the last one is used.
 *
 * @param[out] upcb pointer to a UDP handler
 * @param[in] send_prio_num Number of priorities buffer for the sender queue
 * @param[in] send_queues Priority queues for udp sending.
 * @param[in] receiver_callback Callback for receivering packets
 *
 * @Note Amount of queues in send_queues has to be the same as send_prio_num
 * @return result_t
 */
result_t udp_client_init(struct udp_pcb **upcb, uint8_t send_prio_num,
                         QueueHandle_t *send_queues,
                         receive_callback_t receiver_callback);

#endif // !ETHERNET_UDP
