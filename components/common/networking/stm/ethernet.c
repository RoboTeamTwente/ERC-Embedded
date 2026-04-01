#include "ethernet.h"
#include "api_msg.h"
#include "ethernet_diagnostics.h"
#include "ethernet_raw.h"
#include "ethernet_udp.h"
#include "logging.h"
#include "lwip.h"
#include "queue.h"
#include "tim.h"
#include "udp.h"
#include <stdint.h>
#include <string.h>

#define TAG "MAIN"

extern ETH_HandleTypeDef heth;
extern struct netif gnetif;

struct udp_pcb *upcb;

void ETH_udp_init(uint8_t sender_prio_buf, QueueHandle_t *send_queues,
                  receive_callback_t receiver_callback) {
  udp_client_init(&upcb, sender_prio_buf, send_queues, receiver_callback);
  osDelay(3000); // TODO: very ugly but udp doesn't
                 // start right after the init
}
void ETH_raw_init(raw_receiver_callback callback) { raw_init(callback); }
void ETH_udp_send(uint8_t ip[4], uint8_t port, uint8_t *payload,
                  uint16_t payload_len, uint8_t prio_num,
                  QueueHandle_t *send_queues) {
  udp_client_send(upcb, ip, port, payload, payload_len, prio_num);
}

void ETH_raw_send(uint8_t *mac, char *payload) {
  raw_packet_send(&gnetif, &heth, mac, payload);
}

void ETH_setup_MAC_address_filtering(int mac1[6], int mac2[6], int mac3[6]) {
  ETH_MACFilterConfigTypeDef macfilterconfig;
  HAL_ETH_GetMACFilterConfig(&heth, &macfilterconfig);
  macfilterconfig.HachOrPerfectFilter = DISABLE;
  macfilterconfig.PromiscuousMode = DISABLE;
  HAL_ETH_SetMACFilterConfig(&heth, &macfilterconfig);

  if (mac1 != NULL) {
    (&heth)->Instance->MACA1HR = (1U << 31) | (mac1[5] << 8) | mac1[4];
    (&heth)->Instance->MACA1LR =
        (mac1[3] << 24) | (mac1[2] << 16) | (mac1[1] << 8) | mac1[0];
  }
  if (mac2 != NULL) {
    (&heth)->Instance->MACA2HR = (1U << 31) | (mac2[5] << 8) | mac2[4];
    (&heth)->Instance->MACA2LR =
        (mac2[3] << 24) | (mac2[2] << 16) | (mac2[1] << 8) | mac2[0];
  }
  if (mac3 != NULL) {
    (&heth)->Instance->MACA3HR = (1U << 31) | (mac3[5] << 8) | mac3[4];
    (&heth)->Instance->MACA3LR =
        (mac3[3] << 24) | (mac3[2] << 16) | (mac3[1] << 8) | mac3[0];
  }
}

result_t ETH_add_arp(uint8_t ip[4], uint8_t mac[6]) {
  ip4_addr_t ipaddr;
  struct eth_addr macaddr;

  IP4_ADDR(&ipaddr, ip[0], ip[1], ip[2], ip[3]);
  memcpy(macaddr.addr, mac, 6);

  err_t err = etharp_add_static_entry(&ipaddr, &macaddr);
  if (err == ERR_OK) {
    LOGI(TAG, "Static ARP entry added successfully with IP: %u.%u.%u.%u",
         (uint8_t)ipaddr.addr, (uint8_t)(ipaddr.addr >> 8),
         (uint8_t)(ipaddr.addr >> 16), (uint8_t)(ipaddr.addr >> 24));
    return RESULT_OK;
  } else {
    LOGE(TAG, "Failed to add static ARP entry: %d\n", result_to_short_str(err));
    return RESULT_ERR_COMMS;
  }
}

void ETH_init(
    linkstatus_callback_t link_state_change_callback) { // TODO: return an error
  LOGI(TAG, "Setting up ethernet...\n");
  MX_LWIP_Init();
  ETH_diagnostic_callback_init(&gnetif, link_state_change_callback);
  HAL_ETH_Start_IT(&heth);

  LOGI(TAG, "Ethernet is set up!\n");
}
