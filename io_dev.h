#include "lwip/include/lwip/netif.h"
#include "lwip/include/lwip/tcpip.h"
#include "lwip/include/ipv4/lwip/ip.h"
#include "lwip/include/ipv4/lwip/ip_addr.h"
#include "lwip/include/netif/ethernetif.h"
#include "lwip/include/netif/etharp.h"

struct io_dev {
    uint pa;                    // starting physical address
    uint size;
    uint pte_flags;
    volatile uint *va;          // memory-mapped virtual address
    struct ip_addr ip;          
    struct ip_addr netmask;
    struct ip_addr gw;
    struct netif *netif;
    uint8_t irq;
    struct eth_addr *ethaddr;
    struct ethernetif *eif;
};
