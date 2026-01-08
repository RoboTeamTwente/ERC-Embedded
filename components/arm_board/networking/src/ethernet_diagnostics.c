#include "ethernet_diagnostics.h"
#include "ethernetif.h"
#include "logging.h"
#include <time.h>

#define TAG "ethernet_diagnostics"

extern struct netif gnetif;


void ethernet_linkstatus_callback_default(struct netif *netif) {
  if (netif_is_up(netif)) {
    LOGI(TAG, "Physical ethernet link is up");
  } else {
    LOGE(TAG, "Physical ethernet link is down");
  }
}

void ETH_diagnostic_callback_init(struct netif *netif,
                                  linkstatus_callback_t callback) {
  if (callback != NULL) {
    netif_set_link_callback(netif, callback);
    return;
  }

  netif_set_link_callback(netif, ethernet_linkstatus_callback_default);
}
