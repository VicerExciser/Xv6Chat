/* 	
For details on how to override default lwIP configs here:    
	- https://lwip.fandom.com/wiki/Lwipopts.h

For the full listing of settable options:    
	- lwip/include/lwip/opt.h

Tuning knobs for optimized TCP:    
	- http://lists.gnu.org/archive/html/lwip-users/2006-11/msg00007.html

Probably the best resource, though:    
	- https://www.nongnu.org/lwip/2_0_x/group__lwip__opts.html
*/


/* Enable DHCP (for QEMU's user-mode network stack) */
// #define LWIP_DHCP 1

// #define PPP_SUPPORT 1	// Point-to-Point Protocol

#ifndef LITTLE_ENDIAN
#define LITTLE_ENDIAN 1
#endif

#ifndef BIG_ENDIAN
#define BIG_ENDIAN 2
#endif

#ifndef BYTE_ORDER
#define BYTE_ORDER LITTLE_ENDIAN
#endif

/* MEM_ALIGNMENT: should be set to the alignment of the CPU for which
   lwIP is compiled. 4 byte alignment -> define MEM_ALIGNMENT to 4, 2
   byte alignment -> define MEM_ALIGNMENT to 2. (default: 1) */
#define MEM_ALIGNMENT   4

/* Define IP_FORWARD to 1 if you wish to have the ability to forward
   IP packets across network interfaces. If you are going to run lwIP
   on a device with only one network interface, define this to 0. */
// #define IP_FORWARD      1

/* PBUF_LINK_HLEN: the number of bytes that should be allocated for a
   link level header. (default: 0) */
#define PBUF_LINK_HLEN  14

/* TCP Maximum segment size. */
#define TCP_MSS         512

/* TCP sender buffer space (bytes). (default: 256) */
#define TCP_SND_BUF     2048

/* (default: 2048) */
#define TCP_WND         8192

/* TCP sender buffer space (pbufs). This must be at least = 2 *
   TCP_SND_BUF/TCP_MSS for things to work. 
   (default: 4 * TCP_SND_BUF/TCP_MSS) */
#define TCP_SND_QUEUELEN    (16 * TCP_SND_BUF/TCP_MSS)

/* MEM_SIZE: the size of the heap memory. If the application will send
a lot of data that needs to be copied, this should be set high. (default: 1600) */
#define MEM_SIZE        (4096 * 32) //16)

/* MEMP_NUM_SYS_TIMEOUT: the number of simulateously active timeouts. (default: 3) */
#define MEMP_NUM_SYS_TIMEOUT  10 //16

/* Also used for NUM_SOCKETS in sockets.c (default: 4) */
#define MEMP_NUM_NETCONN   8
#define MEMP_NUM_NETBUF    4

#define MEMP_NUM_SYS_SEM   32 //128
#define MEMP_NUM_SYS_MBOX  15

/* PBUF_POOL_SIZE: the number of buffers in the pbuf pool. (default: 16) */
#define PBUF_POOL_SIZE     128 //256

/* MEMP_NUM_TCP_SEG: the number of simultaneously queued TCP segments. (default: 16) */
#define MEMP_NUM_TCP_SEG    32 //64

#define LWIP_STATS      1

#define LWIP_COMPAT_MUTEX 1
// #define LWIP_COMPAT_SOCKETS 0 	/* (default: 1) */

// #define LWIP_RAW 0
#define LWIP_TCP 1

#define DBG_MIN_LEVEL       0

#define DBG_FULL            DBG_ON | DBG_LEVEL_SEVERE

// #define LWIP_DEBUG          1

#define NETIF_DEBUG         DBG_OFF
#define SYS_DEBUG           DBG_OFF
#define ETHARP_DEBUG        DBG_OFF
#define PBUF_DEBUG          DBG_OFF
#define TCPIP_DEBUG         DBG_OFF //FULL
#define SOCKETS_DEBUG       DBG_OFF //FULL
#define TCP_OUTPUT_DEBUG    DBG_OFF
#define TCP_INPUT_DEBUG     DBG_OFF

// #define TCP_RST_DEBUG       DBG_OFF
#define ICMP_DEBUG          DBG_OFF
#define IP_DEBUG            DBG_OFF
#define UDP_DEBUG           DBG_OFF
#define TCP_DEBUG           DBG_OFF
// #define TCP_QLEN_DEBUG      DBG_OFF
#define DCHP_DEBUG 			DBG_OFF
#define INET_DEBUG 			DBG_OFF
#define API_LIB_DEBUG      DBG_OFF //FULL

#define DBG_TYPES_ON 0xff


