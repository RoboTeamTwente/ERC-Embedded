
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
extern uint8_t IP_ADDRESS[4];

struct udp_pcb *upcb;

ip4_addr_t IPADDR;
ip4_addr_t NETMASK;
ip4_addr_t GW;

void ETH_udp_init(uint8_t sender_prio_num, QueueHandle_t *send_queues,
                  udp_receiver_callback receiver_callback) {
  udp_sender_init(sender_prio_num, send_queues);
  udp_client_init(&upcb, IP_ADDRESS, receiver_callback);
  osDelay(3000); // TODO: very ugly but udp doesn't start right after the init
}
void ETH_custom_protocol_receiver(raw_receiver_callback callback) {
  raw_init(callback);
}
<<<<<<< HEAD

result_t ETH_udp_send(uint8_t ip[4], uint8_t port, const char *payload, uint8_t prio) {
  return udp_client_send_enqueue(ip, port, payload, strlen(payload), prio);
=======
void ETH_udp_send(uint8_t ip[4], uint8_t port, uint8_t *payload,
                  uint16_t payload_len, uint8_t prio_num) {
  udp_client_send(upcb, ip, port, payload, payload_len, prio_num);
>>>>>>> 6e7e4ce13fe4e041a5b9f7ba0ac86a547a058263
}

void ETH_raw_send(uint8_t *mac, char *payload) {
  raw_packet_send(&gnetif, &heth, mac, payload);
}

void ETH_raw_send_binary(uint8_t mac[6], void *payload, size_t length) {
  raw_packet_send_binary(&gnetif, &heth, mac, payload, length);
}

void ETH_setup_MAC_address_filtering(int mac1[6], int mac2[6], int mac3[6]) {
  ETH_MACFilterConfigTypeDef macfilterconfig;
  HAL_ETH_GetMACFilterConfig(&heth, &macfilterconfig);
  macfilterconfig.HachOrPerfectFilter = DISABLE;
  macfilterconfig.PromiscuousMode = ENABLE;  // Enable promiscuous mode for testing
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

result_t ETH_add_arp(uint8_t ip[4], uint8_t mac[6], int retry_count) {
  ip4_addr_t ipaddr;
  struct eth_addr macaddr;

  IP4_ADDR(&ipaddr, ip[0], ip[1], ip[2], ip[3]);
  memcpy(macaddr.addr, mac, 6);

  for (int i = 0; i < retry_count; i++) {

    LOGI(TAG, "Trying to add static ARP entry with IP: %s; try %d",
         ip4addr_ntoa(&ipaddr), i);
    err_t err = etharp_add_static_entry(&ipaddr, &macaddr);
    if (err == ERR_OK) {
      LOGI(TAG, "Static ARP entry added successfully with IP: %s",
           ip4addr_ntoa(&ipaddr));
      return RESULT_OK;
    } else {
      LOGE(TAG, "Failed to add static ARP entry: %d\n",
           result_to_short_str(err));
    }
  }
  return RESULT_ERR_COMMS;
}
// Send a UDP message with binary data using the priority send queue
result_t ETH_udp_send_binary(uint8_t ip[4], uint8_t port, const void *payload, size_t length, uint8_t prio) {
  return udp_client_send_enqueue(ip, port, payload, length, prio);
}

void ETH_address_init(uint8_t ip[4], uint8_t netmask_addr[4],
                      uint8_t gateway[4], uint8_t mac_address[6]) {
  netif_set_down(&gnetif);
  IP4_ADDR(&IPADDR, ip[0], ip[1], ip[2], ip[3]);
  IP4_ADDR(&NETMASK, netmask_addr[0], netmask_addr[1], netmask_addr[2],
           netmask_addr[3]);
  IP4_ADDR(&GW, gateway[0], gateway[1], gateway[2], gateway[3]);

  /* add the network interface (IPv4/IPv6) with RTOS */
  netif_set_ipaddr(&gnetif, &IPADDR);
  netif_set_netmask(&gnetif, &NETMASK);
  netif_set_gw(&gnetif, &GW);

  heth.Init.MACAddr = &mac_address[0];

  gnetif.hwaddr[0] = heth.Init.MACAddr[0];
  gnetif.hwaddr[1] = heth.Init.MACAddr[1];
  gnetif.hwaddr[2] = heth.Init.MACAddr[2];
  gnetif.hwaddr[3] = heth.Init.MACAddr[3];
  gnetif.hwaddr[4] = heth.Init.MACAddr[4];
  gnetif.hwaddr[5] = heth.Init.MACAddr[5];

  HAL_ETH_SetSourceMACAddrMatch(&heth, 0, mac_address);
  netif_set_up(&gnetif);

}

result_t ETH_init(linkstatus_callback_t link_state_change_callback,
                           uint8_t ip[4], uint8_t netmask[4],
                           uint8_t gateway[4],
                           uint8_t mac_address[6]) { // TODO: return an error
  LOGI(TAG, "Setting up ethernet...\n");
  MX_LWIP_Init();
  ETH_address_init(ip, netmask, gateway, mac_address);

  ETH_diagnostic_callback_init(&gnetif, link_state_change_callback);
  uint32_t err = HAL_ETH_GetError(&heth);
  HAL_StatusTypeDef state = HAL_ETH_GetState(&heth);
  for (int i = 0; i < 5; i++){
    if(state != HAL_ETH_STATE_BUSY){break;}
    LOGI(TAG, "Waiting for ethernet to start...");
    osDelay(100);
  }
  if (err != HAL_ETH_ERROR_NONE && state == HAL_ETH_STATE_STARTED || state == HAL_ETH_STATE_BUSY) {
       LOGE(TAG, "Ethernet did not start. Error %d; State %d", err, state);
       return RESULT_ERR_COMMS;
  }

  LOGI(TAG, "Ethernet is set up!\n");
  return RESULT_OK;
}
