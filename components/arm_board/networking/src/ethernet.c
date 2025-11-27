#include "ethernet.h"
#include "ethernet_diagnostics.h"
#include "ethernet_raw_sender.h"
#include "ethernet_receiver.h"
#include "ethernet_udp_sender.h"
#include "logging.h"
#include "lwip.h"
#include "tim.h"
#include "timer_lib.h"
#include "udp.h"
#include <stdint.h>
#define TAG "MAIN"

extern ETH_HandleTypeDef heth;
extern struct netif gnetif;
extern uint8_t IP_ADDRESS[4];

receiver_callback r_callback;
struct udp_pcb *upcb;

void HAL_ETH_RxCpltCallback(ETH_HandleTypeDef *heth) {
  if (r_callback != NULL) {
    ETH_input_callback(heth, r_callback);
    return;
  }

  ETH_input_callback(heth, *ETH_input_callback_example);
  LOGI(TAG, "heth.DMAErrorCode input: %x", heth->DMAErrorCode);

}

void ETH_udp_init() {
  udp_client_init(&upcb, IP_ADDRESS);
  HAL_Delay(3000); // FIXME: very ugly but udp doesn't start right after the init
}

void ETH_udp_send(uint8_t ip[4], uint8_t port, char *payload) {
  udp_client_send(upcb, ip, port, payload);

}

void ETH_raw_send(uint8_t *mac, char *payload) {
  raw_packet_send(&gnetif, &heth, mac, payload);

}

void ETH_init(
    TIM_HandleTypeDef *htim, receiver_callback receiver_callback,
    linkstatus_callback_t link_state_change_callback) { // TODO: return an error
  LOGI(TAG, "Setting up ethernet...\n");
  r_callback = receiver_callback;

  MX_LWIP_Init();

  ETH_diagnostic_callback_init(&gnetif, link_state_change_callback);
  TIM_add_callback(&ETH_diagnostic_checks, htim);
  HAL_TIM_Base_Start_IT(htim);
  HAL_ETH_Start_IT(&heth);
  LOGI(TAG, "Ethernet is set up!\n");
}
