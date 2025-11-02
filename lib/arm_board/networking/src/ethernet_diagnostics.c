#include "ethernet_diagnostics.h"
#include "ethernetif.h"
#include "logging.h"
#include <time.h>

#define TAG "ethernet_diagnostics"

extern struct netif gnetif;
/**
 * @brief does diagnostic checks.
 *          - Checking physical link state
 *
 */
void ETH_diagnostic_checks() { ethernet_link_check_state(&gnetif); }

/**
 * @brief Is called when the physical link status is updated
 *
 * @param[in] netif Pointer to the network handler
 */
void ethernet_linkstatus_callback_default(struct netif *netif) {
  if (netif_is_up(netif)) {
    LOGI(TAG, "Physical ethernet link is up");
  } else {
    LOGE(TAG, "Physical ethernet link is down");
  }
}

/**
 * @brief initializes the diagnostics callbacks
 *
 * @param[in] netif pointer to the network handler
 */
void ETH_diagnostic_callback_init(struct netif *netif,
                                  linkstatus_callback_t callback) {
  if (callback != NULL) {
    netif_set_link_callback(netif, callback);
    return;
  }

  netif_set_link_callback(netif, ethernet_linkstatus_callback_default);
}
