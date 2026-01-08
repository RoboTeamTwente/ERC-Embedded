#include "ethernet.h"
#include "ethernet_diagnostics.h"
#include "ethernet_raw_sender.h"
#include "ethernet_receiver.h"
#include "ethernet_udp_sender.h"
#include "logging.h"
#include "lwip.h"
#include "tim.h"
#include "udp.h"
#include <stdint.h>
#define TAG "MAIN"

extern ETH_HandleTypeDef heth;
extern struct netif gnetif;
extern uint8_t IP_ADDRESS[4];

receiver_callback r_callback;
struct udp_pcb *upcb;

void ETH_udp_init() {
  udp_client_init(&upcb, IP_ADDRESS);
  osDelay(3000); // TODO: very ugly but udp doesn't
                 // start right after the init
}

void ETH_udp_send(uint8_t ip[4], uint8_t port, char *payload) {
  udp_client_send(upcb, ip, port, payload);
}

void ETH_raw_send(uint8_t *mac, char *payload) {
  raw_packet_send(&gnetif, &heth, mac, payload);
}


void ETH_setup_MAC_address_filtering(int mac1[6], int mac2[6], int mac3[6]){
  ETH_MACFilterConfigTypeDef macfilterconfig;
  HAL_ETH_GetMACFilterConfig(&heth, &macfilterconfig);
  macfilterconfig.HachOrPerfectFilter = DISABLE; 
  macfilterconfig.PromiscuousMode = DISABLE;
  HAL_ETH_SetMACFilterConfig(&heth, &macfilterconfig); 

  if(mac1 != NULL){
    (&heth) -> Instance->MACA1HR = (1U << 31) | (mac1[5] << 8) | mac1[4];
    (&heth) -> Instance->MACA1LR = (mac1[3] << 24) |
                 (mac1[2] << 16) |
                (mac1[1] << 8)  |
                mac1[0];
  }
  if(mac2 != NULL){
    (&heth) -> Instance->MACA2HR = (1U << 31) | (mac2[5] << 8) | mac2[4];
    (&heth) -> Instance->MACA2LR = (mac2[3] << 24) |
                (mac2[2] << 16) |
                (mac2[1] << 8)  |
                mac2[0];
  }
  if(mac3 != NULL){
    (&heth) -> Instance->MACA3HR = (1U << 31) | (mac3[5] << 8) | mac3[4];
    (&heth) -> Instance->MACA3LR = (mac3[3] << 24) |
                (mac3[2] << 16) |
                (mac3[1] << 8)  |
                mac3[0];
  }
}

void ETH_init(
    receiver_callback receiver_callback,
    linkstatus_callback_t link_state_change_callback) { // TODO: return an error
  LOGI(TAG, "Setting up ethernet...\n");
  MX_LWIP_Init();
  ETH_diagnostic_callback_init(&gnetif, link_state_change_callback);
  HAL_ETH_Start_IT(&heth);
  ETH_set_receiver_callback( receiver_callback);
  LOGI(TAG, "Ethernet is set up!\n");
}
