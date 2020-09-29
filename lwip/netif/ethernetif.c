/*
 * Copyright (c) 2001-2004 Swedish Institute of Computer Science.
 * All rights reserved. 
 * 
 * Redistribution and use in source and binary forms, with or without modification, 
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission. 
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED 
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF 
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT 
 * SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, 
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT 
 * OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS 
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN 
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING 
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY 
 * OF SUCH DAMAGE.
 *
 * This file is part of the lwIP TCP/IP stack.
 * 
 * Author: Adam Dunkels <adam@sics.se>
 *
 */

/*
 * This file is a skeleton for developing Ethernet network interface
 * drivers for lwIP. Add code to the low_level functions and do a
 * search-and-replace for the word "ethernetif" to replace it with
 * something that better describes your network interface.
 */

#include "lwip/opt.h"
#include "lwip/def.h"
#include "lwip/mem.h"
#include "lwip/pbuf.h"
#include "lwip/sys.h"
#include "lwip/stats.h"
#include "../../pci.h"
#include "netif/ethernetif.h"

#include "netif/etharp.h"

#include "../../string.h"
#include "../../stdio.h"

/* Define those to better describe your network interface. */
#define IFNAME0 'e'
#define IFNAME1 'n'

static const struct eth_addr ethbroadcast = {{0xff,0xff,0xff,0xff,0xff,0xff}};

/* Forward declarations. */
void  ethernetif_input(struct netif *netif);
static err_t ethernetif_output(struct netif *netif, struct pbuf *p,
             struct ip_addr *ipaddr);


static void
low_level_init(struct netif *netif)
{
  struct ethernetif *ethernetif;

  ethernetif = netif->state;
  
  /* set MAC hardware address length */
//  netif->hwaddr_len = 6;

  /* set MAC hardware address */
//  char ethaddr[6];

/*  netif->hwaddr[0] = 0x52;
  netif->hwaddr[1] = 0x54;
  netif->hwaddr[2] = 0x00;
  netif->hwaddr[3] = 0x12;
  netif->hwaddr[4] = 0x34;
  netif->hwaddr[5] = 0x56;*/

  /* maximum transfer unit */
  netif->mtu = 1500;
  
  /* broadcast capability */
  netif->flags = NETIF_FLAG_BROADCAST;
  // netif->flags |= NETIF_FLAG_ETHARP;
 
  /* Do whatever else is needed to initialize interface. */  
}

/*
 * low_level_output():
 *
 * Should do the actual transmission of the packet. The packet is
 * contained in the pbuf that is passed to the function. This pbuf
 * might be chained.
 *
 */

static err_t
low_level_output(struct ethernetif *ethernetif, struct pbuf *p)
{
  struct pbuf *q;
  int totlen = 0;
  unsigned char data[1600];
//  initiate transfer();
  
  if (!ethernetif->send)
      return -2;
  for(q = p; q != NULL; q = q->next) {
    /* Send the data from the pbuf to the interface, one pbuf at a
       time. The size of the data in each pbuf is kept in the ->len
       variable. */
//    send data from(q->payload, q->len);
    // memcpy(data + totlen, q->payload, q->len);
    memmove(data + totlen, q->payload, q->len);
    totlen += q->len;
  }

  ethernetif->send(data, totlen);
//  signal that packet should be sent();
  LWIP_DEBUGF(NETIF_DEBUG,("[low_level_output] ethernetif sent a packet!\n"));
  
#ifdef LINK_STATS
  lwip_stats.link.xmit++;
#endif /* LINK_STATS */      

  return ERR_OK;
}

static err_t
low_level_output_wrap(struct netif *netif, struct pbuf *p)
{
    return low_level_output(netif->state, p);
}

/*
 * low_level_input():
 *
 * Should allocate a pbuf and transfer the bytes of the incoming
 * packet from the interface into the pbuf.
 *
 */

static struct pbuf *
low_level_input(struct ethernetif *ethernetif)
{
  struct pbuf *p, *q;
  u16_t len;
  unsigned char data[1600];
  u16_t curpos = 0;
  u16_t left;
//  int i;

//  cprintf("lwip :: low_level_input\n");
  /* Obtain the size of the packet and put it into the "len"
     variable. */
  len = ethernetif->receive(data, sizeof(data));
/*  cprintf(" read %d bytes\n", len);
  for (left = 0; left < len; left ++)
  {
      cprintf("%02x ", data[left]);
      if ((left + 1) % 16 == 0)
          cprintf("\n");
  }
  cprintf("\n");*/

  if (len <= 0)
    return NULL;

  LWIP_DEBUGF(NETIF_DEBUG,("[low_level_input] ethernetif received a packet!\n"));

  /* We allocate a pbuf chain of pbufs from the pool. */
  p = pbuf_alloc(PBUF_RAW, len, PBUF_POOL);
  
  if (p != NULL) {
    /* We iterate over the pbuf chain until we have read the entire
       packet into the pbuf. */
    for(q = p; q != NULL; q = q->next) {
      /* Read enough bytes to fill this pbuf in the chain. The
         available data in the pbuf is given by the q->len
         variable. */
//      read data into(q->payload, q->len);
//        len = ether_receive(q->payload, q->len);
        if (q->len > len - curpos)
            left = len - curpos;
        else
            left = q->len;
        // memcpy(q->payload, data + curpos, left);
        memmove(q->payload, data + curpos, left);
        curpos += left;
    }
//    acknowledge that packet has been read();
#ifdef LINK_STATS
    lwip_stats.link.recv++;
#endif /* LINK_STATS */      
  } else {
//    drop packet();
#ifdef LINK_STATS
    lwip_stats.link.memerr++;
    lwip_stats.link.drop++;
    cprintf("low_level_input: no mem\n");
#endif /* LINK_STATS */      
  }

  return p;  
}

/*
 * ethernetif_output():
 *
 * This function is called by the TCP/IP stack when an IP packet
 * should be sent. It calls the function called low_level_output() to
 * do the actual transmission of the packet.
 *
 */

static err_t
ethernetif_output(struct netif *netif, struct pbuf *p,
      struct ip_addr *ipaddr)
{
  struct ethernetif *ethernetif;
//  struct pbuf *q;
//  struct eth_hdr *ethhdr;
//  struct eth_addr *dest, mcastaddr;
//  struct ip_addr *queryaddr;
//  err_t err;
//  u8_t i;
  
  ethernetif = netif->state;

  /* resolve the link destination hardware address */
  p = etharp_output(netif, ipaddr, p);
  LWIP_DEBUGF(NETIF_DEBUG,("[output] ethernetif sent via etharp_output\n"));
  
  /* network hardware address obtained? */
  if (p == NULL)
  {
    /* we cannot tell if the packet was sent: the packet could */
    /* have been queued on an ARP entry that was already pending. */
  	return ERR_OK;
  }
  	
  /* send out the packet */
  return low_level_output(ethernetif, p);

}

/*
 * ethernetif_input():
 *
 * This function should be called when a packet is ready to be read
 * from the interface. It uses the function low_level_input() that
 * should handle the actual reception of bytes from the network
 * interface.
 *
 */

void
ethernetif_input(struct netif *netif)
{
  struct ethernetif *ethernetif;
  struct eth_hdr *ethhdr;
  struct pbuf *p, *q;

  ethernetif = netif->state;
  
  p = low_level_input(ethernetif);

  if (p == NULL)
    return;

#ifdef LINK_STATS
  lwip_stats.link.recv++;
#endif /* LINK_STATS */

  ethhdr = p->payload;
  q = NULL;

  switch (htons(ethhdr->type)) {
    case ETHTYPE_IP:
      LWIP_DEBUGF(NETIF_DEBUG,("[input] ethernetif received ETHTYPE_IP\n"));
      q = etharp_ip_input(netif, p);
      pbuf_header(p, -14);
      netif->input(p, netif);
      break;
      
    case ETHTYPE_ARP:
      LWIP_DEBUGF(NETIF_DEBUG,("[input] ethernetif received ETHTYPE_ARP\n"));
      q = etharp_arp_input(netif, ethernetif->ethaddr, p);
      break;
    default:
      LWIP_DEBUGF(NETIF_DEBUG,("[input] ethernetif received ETHTYPE_UNKNOWN\n"));
      pbuf_free(p);
      p = NULL;
      break;
  }
  if (q != NULL) {
    low_level_output(ethernetif, q);
    pbuf_free(q);
    q = NULL;
  }
}
// #include "../../x86.h"
// extern void e1000_intr(struct trapframe *tf);
static void
arp_timer(void *arg)
{
  etharp_tmr();
  // cprintf("arp_timer: etharp_tmr updated\n");
  LWIP_DEBUGF(NETIF_DEBUG, ("arp_timer: etharp_tmr updated\n"));
  sys_timeout(ARP_TMR_INTERVAL, arp_timer, NULL);
}

/*
 * ethernetif_init():
 *
 * Should be called at the beginning of the program to set up the
 * network interface. It calls the function low_level_init() to do the
 * actual setup of the hardware.
 *
 */

err_t
ethernetif_init(struct netif *netif)
{
  struct ethernetif *ethernetif;
    
  ethernetif = mem_malloc(sizeof(struct ethernetif));
  
  if (ethernetif == NULL)
  {
  	LWIP_DEBUGF(NETIF_DEBUG, ("ethernetif_init: out of memory\n"));
  	return ERR_MEM;
  }
  
  netif->state = ethernetif;
  // cprintf("[ethernetif_init] netif (0x%x) state = ethernetif (0x%x)\n",
  //     netif, ethernetif);
  netif->name[0] = IFNAME0;
  netif->name[1] = IFNAME1;
  netif->output = ethernetif_output;
  netif->linkoutput = low_level_output_wrap;
  
  ethernetif->ethaddr = (struct eth_addr *)&(netif->hwaddr[0]);
  
  low_level_init(netif);

  etharp_init();

  sys_timeout(ARP_TMR_INTERVAL, arp_timer, NULL);
  // sys_timeout(0xffffffff, arp_timer, NULL);
  return 0;
}
