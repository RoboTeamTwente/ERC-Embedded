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

ip4_addr_t IPADDR;
ip4_addr_t NETMASK;
ip4_addr_t GW;

void ETH_udp_init(uint8_t sender_prio_buf, QueueHandle_t *send_queues,
                  receive_callback_t receiver_callback) {
  udp_client_init(&upcb, sender_prio_buf, send_queues, receiver_callback);
  osDelay(3000); // TODO: very ugly but udp doesn't
                 // start right after the init
}
void ETH_custom_protocol_receiver(raw_receiver_callback callback) {
  raw_init(callback);
}
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
}

HAL_StatusTypeDef ETH_init(linkstatus_callback_t link_state_change_callback,
                           uint8_t ip[4], uint8_t netmask[4],
                           uint8_t gateway[4],
                           uint8_t mac_address[6]) { // TODO: return an error
  LOGI(TAG, "Setting up ethernet...\n");
  MX_LWIP_Init();
  ETH_address_init(ip, netmask, gateway, mac_address);

  ETH_diagnostic_callback_init(&gnetif, link_state_change_callback);
  HAL_StatusTypeDef err = HAL_ETH_Start_IT(&heth);
  if (err != ERR_OK) {
    LOGE(TAG, "Cannot start ethernet");
    return err;
  }
  LOGI(TAG, "Ethernet is set up!\n");
  return err;
}
