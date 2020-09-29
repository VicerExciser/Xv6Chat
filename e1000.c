#include "types.h"
#include "mmu.h"
#include "e1000.h"
#include "proc.h"
#include "defs.h"
// #include "io_dev.h"
#include "memlayout.h"
#include "nic.h"
#include "traps.h"

#include "lwip/opt.h"

#if LWIP_DHCP
#include "lwip/include/lwip/dhcp.h"
#endif

#include "thread.h"
#include "picirq.h"

// LAB 6: Your driver code here

/*  Be mindful of the alignment requirements on the transmit descriptor array and the restrictions on length of this array.
    
    Since TDLEN must be 128-byte aligned and each transmit descriptor is 16 bytes, 
    your transmit descriptor array will need some multiple of 8 transmit descriptors. 
    
    However, don’t use more than 64 descriptors or our tests won’t be able to test transmit ring overflow.

    Keep in mind that the E1000 accesses physical memory directly, which means any buffer it accesses
    must be contiguous in physical memory.

    Both descriptor queues must be aligned on a paragraph (16-byte) boundary.
*/

// Descriptor Ring Buffers
struct tdesc e1000_tx_queue[TXRING_LEN] __attribute__((aligned (16)));
struct rdesc e1000_rx_queue[RXRING_LEN] __attribute__((aligned (16)));

// Packet Buffers
struct tx_pkt e1000_tx_pktbuf[TXRING_LEN];
struct rx_pkt e1000_rx_pktbuf[RXRING_LEN];

// Hardcoded MAC Address Values
// uint8_t e1000_mac[6] = { 0x52, 0x54, 0x00, 0x12, 0x34, 0x56 };   // QEMU's default MAC address (a la https://en.wikibooks.org/wiki/QEMU/Networking)
// uint8_t e1000_mac[6] = { 0x12, 0x13, 0x14, 0x15, 0x16, 0x27 };   // QEMUSERV MAC address as specified in the Makefile
// uint8_t e1000_mac[6] = { 0x12, 0x13, 0x14, 0x15, 0x16, 0x17 };   // QEMUCLIENT MAC address as specified in the Makefile

struct netif e1000_netif;
struct eth_addr e1000_hwaddr;
// struct ethernetif e1000_eif;
struct nic_device e1000_nic;

// struct io_dev {
//     uint pa;        // starting physical address
//     uint size;
//     uint pte_flags;
//     volatile uint *va;          // MMIO address for the E1000's BAR
//     struct ip_addr ip;   // these three are included in the netif structure
//     struct ip_addr netmask;
//     struct ip_addr gw;
//     struct netif *netif;
//     uint8_t irq;
//     struct eth_addr *ethaddr;
//     struct ethernetif *eif;
// };

// struct io_dev e1000;


// Global NIC device structure necessary for registering the E1000's hardware address with the kernel's arp handler
// struct nic_device nic;

// Global structure (defined in mmu.h) for tracking the E1000's memory-mapping metadata
// struct dev_vm_map e1000_vmap;

// struct ip_addr ipaddr, netmask, gw;

// uint8_t irq;


static void init_tx(void);
static void init_rx(void);


int
e1000_attach(struct pci_func *pcif)
{
    uint i;
    memset(&e1000, 0, sizeof(e1000));

    // Enable PCI device
    pci_func_enable(pcif);
    
    e1000.pa = pcif->reg_base[0];
    e1000.size = pcif->reg_size[0];
    e1000.pte_flags = PTE_W | PTE_PWT | PTE_PCD;
    e1000.irq = pcif->irq_line;

    cprintf("\nE1000's BAR 0 physaddr:0x%x, size:0x%x, irq:%u\n", e1000.pa, e1000.size, e1000.irq);    // PA assigned to 0xfebc0000, above 4GB

    // Create virtual memory mapping in the kernel page table
    e1000.va = (uint *)mmio_map_region(e1000.pa, e1000.size);
    cprintf("E1000 now mapped to virtual address 0x%x\n", e1000.va);

    // Verify status -- You should get 0x80080783, which indicates a full duplex link is up at 1000 MB/s, among other things.
    assert(e1000.va[E1000_STATUS] == 0x80080783);
    cprintf("E1000_STATUS = 0x%x  (should be 0x80080783)\n", e1000.va[E1000_STATUS]);

    e1000.netif = &e1000_netif;
    e1000.ethaddr = &e1000_hwaddr;

    // Finish NIC initialization process for packet Transmit & Receive 
    init_tx();
    init_rx();

    e1000.netif->hwaddr_len = 6;
    for (i = 0; i < 6; i++) {
        e1000_nic.mac_addr[i]  = e1000.ethaddr->addr[i];
        e1000.netif->hwaddr[i] = e1000.ethaddr->addr[i];
    }

    // Be sure to register the device MAC address with the kernel's arp handler. See arp.c for details.
    cprintf("E1000 init_rx configured NIC's MAC address: '%02x:%02x:%02x:%02x:%02x:%02x'\n", 
            e1000_nic.mac_addr[0], 
            e1000_nic.mac_addr[1],
            e1000_nic.mac_addr[2],
            e1000_nic.mac_addr[3],
            e1000_nic.mac_addr[4],
            e1000_nic.mac_addr[5]);
    register_device(e1000_nic);

#if !LWIP_DHCP

    if (e1000.ethaddr->addr[5] == 0x27)            // Then this is the server
    // {                                       // Set static IP address 10.0.2.17 for server    
    //     IP4_ADDR(&e1000.ip, 10, 0, 2, 17);
    //     cprintf("[serv] E1000 assigned IPv4 address: '%d.%d.%d.%d'\n\n", 10, 0, 2, 17); 
    {                                       // Set static IP address 10.0.3.17 for server    
        IP4_ADDR(&e1000.ip, 10, 0, 3, 17);
        cprintf("[serv] E1000 assigned IPv4 address: '%d.%d.%d.%d'\n\n", 10, 0, 3, 17); 
    } 
    else if (e1000.ethaddr->addr[5] == 0x17)       // Then this is the local client
    {                                       // Set static IP address 10.0.3.16 for local client
        IP4_ADDR(&e1000.ip, 10, 0, 3, 16);
        cprintf("[client] E1000 assigned IPv4 address: '%d.%d.%d.%d'\n\n", 10, 0, 3, 16); 
    }
    else if (e1000.ethaddr->addr[5] == 0x37)       // Then this is the local client
    {                                       // Set static IP address 10.0.3.18 for local client
        IP4_ADDR(&e1000.ip, 10, 0, 3, 18);
        cprintf("[client] E1000 assigned IPv4 address: '%d.%d.%d.%d'\n\n", 10, 0, 3, 18); 
    }else if (e1000.ethaddr->addr[5] == 0x47)       // Then this is the local client
    {                                       // Set static IP address 10.0.3.19 for local client
        IP4_ADDR(&e1000.ip, 10, 0, 3, 19);
        cprintf("[client] E1000 assigned IPv4 address: '%d.%d.%d.%d'\n\n", 10, 0, 3, 19); 
    }

#endif
    /*  If running in user-mode, the guest OS will see an E1000 NIC with a virtual DHCP
        server on 10.0.2.2 and will be allocated an address starting from 10.0.2.15.
        A virtual DNS server will be accessible on 10.0.2.3.
        Will only support TCP and UDP.
        By default, the DHCP server acts as a firewall and does no permit an incoming traffic.
    */

    IP4_ADDR(&e1000.netmask, 255, 255, 255, 0);
    IP4_ADDR(&e1000.gw, 10, 0, 3, 2);       // <-- enp0s8 NAT interface


    /*  Files to see regarding the following initialization chains:
            lwip/core/netif.c
            lwip/netif/ethernetif.c

        netif_add --> ethernetif_init --> low_level_init
    */

    /*  Eligible my_netif->input functions to pass to netif_add:

        - If Non-Ethernet netif (e.g. PPP or slipif):
            err_t ip_input(struct pbuf *p, struct netif *inp);  <-- lwip/ip.h
        ^ Link headers must be removed (pbuf->payload must point to the IP header)
        
        - tcpip_input   <-- Will handle all supported message types
            
    */
    netif_add(e1000.netif, &e1000.ip, &e1000.netmask, &e1000.gw, NULL, ethernetif_init, tcpip_input);
    // netif_add(e1000.netif, &e1000.ip, &e1000.netmask, &e1000.gw, NULL, ethernetif_init, ip_input);

    // ethernetif_init will mem_malloc the e1000's struct eithernetif *eif (returned as the netif's state variable),
    // as well as set the ethaddr to that of the netif
        // struct ethernetif {
        //     struct eth_addr *ethaddr;
        //      Add whatever per-interface state that is needed here. 
        //       int (*send)(void *data, uint32_t len);
        //       int (*receive)(void *data, uint32_t len);
        // };
    e1000.eif = e1000.netif->state;
    assert(e1000.eif != NULL);

    e1000.netif->mtu = TXPKT_SIZE;  // Override max packet size that was set in low_level_init()
    // e1000.netif->flags |= NETIF_FLAG_ETHARP;    // Set flag to signify this interface uses ARP

    /* These were set in ethernetif_init:
        netif->output = ethernetif_output;
        netif->linkoutput = low_level_output_wrap;
    */

    // NOTE: THESE TWO LINES ARE  C R U C I A L
    // IPCs are sent by low_level_output() in ethernetif.c, which is the mechanism responsible for sending
    // packets via the specified ethernetif->send function pointer.
    // Setting this to our driver's e1000_transmit system call is what glues the lwIP stack to xv6's network system!!
    e1000.eif->send = e1000_transmit;
    e1000.eif->receive = e1000_receive;

    // Register interrupt handler
    reg_irq_handler(e1000.irq, e1000_intr);

    // Enable Interrupt Request Line
    picenable((ushort)e1000.irq);   
    ioapicenable(e1000.irq, 0);
    // for (i = 0; i < ncpu; i++)
        ioapicenable(e1000.irq, i);


    // Initialize the RX lock (for the "e1000 rx thread")
    initlock(&e1000.rxlock, "e1000_rx");

    netif_set_default(e1000.netif);

    /* if set, the interface has an active link */
    e1000.netif->flags |= NETIF_FLAG_LINK_UP;   

    /** whether the network interface is 'up'. this is
     * a software flag used to control whether this network
     * interface is enabled and processes traffic */
    e1000.netif->flags |= NETIF_FLAG_UP;

#if LWIP_DHCP

    cprintf("[e1000_attach] DHCP enabled\n");
    e1000.netif->flags |= NETIF_FLAG_DHCP;
    dhcp_start(e1000.netif);
    assert(e1000.netif->dhcp != NULL);

    cprintf("E1000 assigned IPv4 address: '%u.%u.%u.%u'\n", 
        ip4_addr1(&e1000.ip),
        ip4_addr2(&e1000.ip),
        ip4_addr3(&e1000.ip),
        ip4_addr4(&e1000.ip));

#endif

    // Launch the Ethernet Receive Thread
    sys_thread_new(e1000_rx_thread, 0, 2, "e1000 rx thread");

    return 0;
}

/**
 * The pointers to the descriptor queues as well as the addresses of the packet buffers in the descriptors must all be 
 * physical addresses because hardware performs DMA directly to and from physical RAM without going through the MMU.
 */
static void
init_tx(void)   /* See section 14.5 of the Intel manual */
{
    uint i;

    memset(e1000_tx_queue, 0, sizeof(e1000_tx_queue));
    memset(e1000_tx_pktbuf, 0, sizeof(e1000_tx_pktbuf));

    for (i = 0; i < TXRING_LEN; i++) {
        e1000_tx_queue[i].addr = (uint64_t)V2P(e1000_tx_pktbuf[i].buf);
        e1000_tx_queue[i].status |= E1000_TXD_STAT_DD;  // Mark each descriptor as ready for new packet
    }

    // Configure the E1000 transmit registers
    e1000.va[E1000_TDBAL] = V2P(e1000_tx_queue);       // TX Descriptor Base Address Low reg
    e1000.va[E1000_TDBAH] = 0;                         // TX Descriptor Base Address High reg
    e1000.va[E1000_TDLEN] = sizeof(e1000_tx_queue);    // TX Descriptor Length reg
    e1000.va[E1000_TDH]   = 0;                         // TX Descriptor Head reg  (hardware-controlled index into e1000_tx_queue)
    e1000.va[E1000_TDT]   = 0;                         // TX Descriptor Tail reg  (software-controlled index into e1000_tx_queue)

    // Configure the TX Control register
    /*              
                    31      25   21      12 11     4 3   0
                    +------+----+----------+--------+----+
        TCTL:       |      |CNTL|   COLD   |   CT   |CNTL|
                    +------+----+----------+--------+----+

        Must set the Enable (TCTL.EN) and Pad Short Packets (TCTL.PSP) bits,
        set the Collision Threshold (TCTL.CT) to Ethernet standard (10h),
        and set the Collision Distance (TCTL.COLD) for full-duplex (40h).
    */
    e1000.va[E1000_TCTL] = 0x0 | (ETH_COLD_FULLDUP << 12) | (ETH_CT_STANDARD << 4) | E1000_TCTL_EN | E1000_TCTL_PSP;

    // Program the Transmit IPG Register with the default values described in table 13-77 of section 13.4.34 for the IEEE 802.3 standard IPG
    /*              
                    31  29     20 19     10 9        0
                    +--+---------+---------+---------+
        TIPG:       |  |  IPGR2  |  IPGR1  |   IPGT  |
                    +------------+---------+---------+

        According to the IEEE 802.3 standard IPG configurations, must set the
        IPG Receive Time 2 (IPGR2) to 6, the IPG Receive Time 1 (IPGR1) to 4,
        and the IPG Transmit Time (IPGT) to 10.
    */
    e1000.va[E1000_TIPG] = 0x0 | (IEEE_IPGR2 << 20) | (IEEE_IPGR1 << 10) | IEEE_IPGT;  
}


static void
init_rx(void)   /* See section 14.4 of the Intel manual */
{
    uint i;

    memset(e1000_rx_queue, 0, sizeof(e1000_rx_queue));
    memset(e1000_rx_pktbuf, 0, sizeof(e1000_rx_pktbuf));
    for (i = 0; i < RXRING_LEN; i++) {
        e1000_rx_queue[i].addr = (uint64_t)V2P(e1000_rx_pktbuf[i].buf);
        e1000_rx_queue[i].status = 0;
    }

    // Read the hardware (MAC) address from the device registers RAL && RAH (configured in Makefile)
    e1000.ethaddr->addr[0] = (uint8_t)((e1000.va[E1000_RAL]) & 0xff); 
    e1000.ethaddr->addr[1] = (uint8_t)((e1000.va[E1000_RAL] >> 8) & 0xff); 
    e1000.ethaddr->addr[2] = (uint8_t)((e1000.va[E1000_RAL] >> 16) & 0xff); 
    e1000.ethaddr->addr[3] = (uint8_t)((e1000.va[E1000_RAL] >> 24) & 0xff);
    e1000.ethaddr->addr[4] = (uint8_t)((e1000.va[E1000_RAH]) & 0xff);
    e1000.ethaddr->addr[5] = (uint8_t)((e1000.va[E1000_RAH] >> 8) & 0xff);

    // Configure the E1000 receive registers
    e1000.va[E1000_RDBAL] = V2P(e1000_rx_queue);       // RX Descriptor Base Address Low reg
    e1000.va[E1000_RDBAH] = 0;                         // RX Descriptor Base Address High reg
    e1000.va[E1000_RDLEN] = sizeof(e1000_rx_queue);    // RX Descriptor Length reg
    e1000.va[E1000_RDH]   = 0;                         // RX Descriptor Head reg  (hardware-controlled index into e1000_rx_queue)
    e1000.va[E1000_RDT]   = 0;                         // RX Descriptor Tail reg  (software-controlled index into e1000_rx_queue)

    // Configure the RX Control Register
    /*
        • Set the receiver Enable (RCTL.EN) bit to 1b for normal operation. However, it is best to leave
        the Ethernet controller receive logic disabled (RCTL.EN = 0b) until after the receive
        descriptor ring has been initialized and software is ready to process received packets.

        • Set the Long Packet Enable (RCTL.LPE) bit to 1b when processing packets greater than the
        standard Ethernet packet size. For example, this bit would be set to 1b when processing Jumbo
        Frames. (optional -- not yet implemented)

        • Set the Broadcast Accept Mode (RCTL.BAM) bit to 1b allowing the hardware to accept
        broadcast packets.

        • Configure the Receive Buffer Size (RCTL.BSIZE) bits to reflect the size of the receive buffers
        software provides to hardware. Also configure the Buffer Extension Size (RCTL.BSEX) bits if
        receive buffer needs to be larger than 2048 bytes. (defaults to 00b = 2048 bytes)

        • Set the Strip Ethernet CRC (RCTL.SECRC) bit if the desire is for hardware to strip the CRC
        prior to DMA-ing the receive packet to host memory.
    */
    e1000.va[E1000_RCTL]  = E1000_RCTL_EN | E1000_RCTL_BAM | E1000_RCTL_SECRC;     // Enable and strip Ethernet CRC field

    // Configure packet RX interrupts -- see sections 3.2.7 and 13.4.20 of the Intel manual
    e1000.va[E1000_RSRPD] = 0x00000fff;     // Configure interrupts for any packet received of size less than 4096

    // The Ethernet controller can generate four RX-related interrupts (if enabled in the Interrupt Mask Set Register):
    e1000.va[E1000_IMS]   = E1000_IMS_SRPD;   // 1. Small Receive Packet Detect (ICR.SRPD)
    e1000.va[E1000_IMS]  |= E1000_IMS_RXT0;   // 2. Receiver Timer Interrupt (ICR.RXT0)
    e1000.va[E1000_IMS]  |= E1000_IMS_RXDMT0; // 3. Receive Descriptor Minimum Threshold (ICR.RXDMT0)
    e1000.va[E1000_IMS]  |= E1000_IMS_RXO;    // 4. Receiver FIFO Overrun (ICR.RXO)
    e1000.va[E1000_IMS]  |= E1000_IMS_LSC;    // (suggested: Link Status Change interrupt)
    e1000.va[E1000_IMS]  |= E1000_IMS_TXDW;   // (optional: Transmit Descriptor Written Back interrupt)

    /*  Other interrupt-related registers...
    e1000.va[E1000_ICR]  -->  Interrupt Cause Read Register: driver can read to discern specific cause/type of interrupt
        (Note: reading the bitfield value from the ICR reg will clear its contents)

    e1000.va[E1000_ICS]  -->  Interrupt Cause Set Register: can set ICS bits to trigger (test) enabled RX interrupts 

    e1000.va[E1000_ICM]  -->  Interrupt Clear Mask Register: used to disable interrupts that have been set in the IMS reg
        (Note: writing a 0b to any set bit in the IMS reg is ineffectual, rather setting
         the corresponding bit mask in the ICM reg is required to disabling interrupts;
         this is primarily for ensuring thread-safe interactions with all interrupt regs)
    */
}

/**
 * To transmit a packet, you have to add it to the tail of the transmit queue, which means copying the packet data into the next 
 * packet buffer and then updating the TDT (transmit descriptor tail) register to inform the card that there’s another packet in 
 * the transmit queue. (Note that TDT is an index into the transmit descriptor array, not a byte offset; the documentation isn’t 
 * very clear about this.)
 * If a descriptor’s DD bit is set, you know it’s safe to recycle that descriptor and use it to transmit another packet.
 */
int
e1000_transmit(void *buf, unsigned int len)
{
    uint tail = e1000.va[E1000_TDT];

    // Check if transmit queue is full
    if ((e1000_tx_queue[tail].status & E1000_TXD_STAT_DD) != E1000_TXD_STAT_DD)
        return -1;      // How should caller handle full TX queue? Drop the packet? Retry? Sleep & wait for interrupt from NIC?

    // Prevent potential buffer overflows
    len %= TXPKT_SIZE;

    // Copy the packet data into the next available packet buffer, update the descriptor, and update TDT reg
    memmove(&e1000_tx_pktbuf[tail], buf, len);
    e1000_tx_queue[tail].length = (uint16_t)len;
    e1000_tx_queue[tail].status &= ~E1000_TXD_STAT_DD;                  // Clear status DD bit (signaling as ready to transmit)
    e1000_tx_queue[tail].cmd |= E1000_TXD_CMD_EOP | E1000_TXD_CMD_RS;   // Set RS bit in TX descriptor's command field to tell
                                                                        //  the card to set the DD status bit once it sends the packet
    e1000.va[E1000_TDT] = ++tail % TXRING_LEN;

    return 0;
}

static uint 
get_rx_tail(void)
{
    uint tail = e1000.va[E1000_RDT];

    // Need to tickle the hardware a bit to trigger a response from the hardware (so that DD bit may get set)
    // Note that immediately writing the same read-in RDT value back to the RDT reg will have no effect
    e1000.va[E1000_RDT] = --tail % RXRING_LEN;
    e1000.va[E1000_RDT] = ++tail % RXRING_LEN;

    return tail;
}

/**
 * When the E1000 receives a packet, it first checks if it matches the card’s configured filters (for example, to see if the packet 
 * is addressed to this E1000’s MAC address) and ignores the packet if it doesn’t match any filters. Otherwise, the E1000 tries to 
 * retrieve the next receive descriptor from the head of the receive queue. If the head (RDH) has caught up with the tail (RDT), 
 * then the receive queue is out of free descriptors, so the card drops the packet. If there is a free receive descriptor, it copies 
 * the packet data into the buffer pointed to by the descriptor, sets the descriptor’s DD (Descriptor Done) and EOP (End of Packet) 
 * status bits, and increments the RDH.
 *
 * If the E1000 receives a packet that is larger than the packet buffer in one receive descriptor, it will retrieve as many descriptors 
 * as necessary from the receive queue to store the entire contents of the packet. To indicate that this has happened, it will set the 
 * DD status bit on all of these descriptors, but only set the EOP status bit on the last of these descriptors. You can either deal with 
 * this possibility in your driver, or simply configure the card to not accept “long packets” (also known as jumbo frames) and make sure 
 * your receive buffers are large enough to store the largest possible standard Ethernet packet (1518 bytes).
 */
int
e1000_receive(void *buf, unsigned int len)
{
    /*  Software can determine if a receive buffer is valid by reading descriptors in memory
        rather than by I/O reads. Any descriptor with a non-zero status byte has been processed by the
        hardware, and is ready to be handled by the software. 

        If software statically allocates buffers, and uses memory read to check for completed descriptors, it
        simply has to zero the status byte in the descriptor to make it ready for reuse by hardware 
    */
    uint tail = get_rx_tail();

    // Proceed only if a packet has been received (DD bit is set) and no errors occurred
    if (e1000_rx_queue[tail].errors)
        return -1;

    if ((e1000_rx_queue[tail].status & E1000_RXD_STAT_DD) != E1000_RXD_STAT_DD)
        return 0;

    if (!(e1000_rx_queue[tail].status & E1000_RXD_STAT_EOP)) {
        // panic("this card does not support jumbo frames");
        cprintf("E1000 RX ERROR: this card does not support jumbo frames!\n");
        return -1;
    }

    // Prevent potential buffer overflows
    len %= RXPKT_SIZE;
    if (e1000_rx_queue[tail].length < len)
        len = e1000_rx_queue[tail].length;

    // Copy the packet data out of the descriptor’s packet buffer
    memmove(buf, e1000_rx_pktbuf[tail].buf, len);

    // Tell the card that the descriptor is free now by updating the queue's tail index and clearing the status bits
    e1000_rx_queue[tail].status = 0;
    e1000.va[E1000_RDT] = ++tail % RXRING_LEN;

    return len;

    // Possible future TODOs:
    //  - Support long packets that span multiple receive buffers

    /*  If the E1000 receives a packet that is larger than the packet buffer in one receive descriptor, 
        it will retrieve as many descriptors as necessary from the receive queue to store the entire contents of the packet. 
        To indicate that this has happened, it will set the DDstatus bit on all of these descriptors, 
        but only set the EOP status bit on the last of these descriptors.
    */
}

static int
check_intr(void)
{
    uint icr = e1000.va[E1000_ICR];

#if E1000_DEBUG

    int intr = 0;
    if ((icr & E1000_ICR_SRPD) == E1000_ICR_SRPD) {
        cprintf("E1000_ICR_SRPD\n");
        intr = 1;
    }
    if ((icr & E1000_ICR_RXT0) == E1000_ICR_RXT0) {
        cprintf("E1000_ICR_RXT0\n");
        intr = 1;
    }
    if ((icr & E1000_ICR_RXO) == E1000_ICR_RXO) {
        cprintf("E1000_ICR_RXO\n");
        intr = 1;
    }
    if ((icr & E1000_ICR_TXDW) == E1000_ICR_TXDW) {
        cprintf("E1000_ICR_TXDW\n");
        intr = 1;
    }
    if ((icr & E1000_ICR_LSC) == E1000_ICR_LSC) {
        cprintf("E1000_ICR_LSC\n");
        intr = 1;
    }
    return intr;

#else

    return     ((icr & E1000_ICR_SRPD) == E1000_ICR_SRPD)
            || ((icr & E1000_ICR_RXT0) == E1000_ICR_RXT0)
            || ((icr & E1000_ICR_RXO)  == E1000_ICR_RXO)
            || ((icr & E1000_ICR_TXDW) == E1000_ICR_TXDW)
            || ((icr & E1000_ICR_LSC)  == E1000_ICR_LSC);

#endif
}

void
e1000_rx_thread(void *arg)
{
    #if E1000_DEBUG 
    cprintf("[e1000_rx_thread] START\n");
    #endif

    acquire(&e1000.rxlock);
    while (1)
    {
        #if E1000_DEBUG 
        cprintf("[e1000_rx_thread] SLEEP\n");
        #endif
        
        sleep(&e1000.rxlock, &e1000.rxlock);    // Will be woken up by an interrupt
        
        #if E1000_DEBUG 
        cprintf("[e1000_rx_thread] WOKE!\n");
        #endif
        
        uint tail = get_rx_tail();
        while ((e1000_rx_queue[tail].status & E1000_RXD_STAT_DD) == E1000_RXD_STAT_DD) 
        {
            ethernetif_input(e1000.netif);
            tail = get_rx_tail();
        }
    }
    release(&e1000.rxlock);
}

void
e1000_intr(struct trapframe *tf)
{
    if (check_intr()) 
    {
        #if E1000_DEBUG 
        cprintf("-- E1000 INTERRUPT HANDLER INVOKED --\n");
        #endif

        wakeup_one(&e1000.rxlock);
    }
}

/**
 * Fire a Link Status Change interrupt to provoke a response from the driver thread
 */
void
e1000_force_intr(void)
{
    e1000.va[E1000_ICS] = E1000_ICS_LSC;
}

