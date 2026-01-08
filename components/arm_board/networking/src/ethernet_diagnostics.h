#ifndef ETHERNET_DIAGNOSTICS
#define ETHERNET_DIAGNOSTICS

#include "netif.h"


/**
 * @brief callback function for when the link status changes
 *
 * @param netif pointer to the network interface
 *
 */
typedef void (*linkstatus_callback_t)(struct netif *netif);

/**
 * @brief Initialization for the callback of diagnostics
 *
 * @param netif network interface
 * @param linkstatus_callback_t Callback function for the linkstatus.
 *                              Can be Null, then a default function is used.
 */
void ETH_diagnostic_callback_init(struct netif *netif,
                                  linkstatus_callback_t callback);

#endif // !ETHERNET_DIAGNOSTICS
