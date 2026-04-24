// #define LWIP_HOOK_UNKNOWN_ETH_PROTOCOL(pbuf, netif) eth_reader(netif, pbuf)
// #define LWIP_DEBUG 1
#define LWIP_HOOK_VLAN_SET(pcb, hdr, netif, src, dst, eth_hdr_len)             \
  get_vlan_header(pcb, hdr, netif, src, dst, eth_hdr_len)
