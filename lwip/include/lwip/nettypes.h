#ifndef __XV6CHAT_NETTYPES_H__
#define __XV6CHAT_NETTYPES_H__
#include "arch.h"
#include "err.h"
////////////////////////////////////////////////////////////////////////////////////////////
//// FROM pbuf.h ///////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////
#define PBUF_TRANSPORT_HLEN 20
#define PBUF_IP_HLEN        20
typedef enum {
  PBUF_TRANSPORT,
  PBUF_IP,
  PBUF_LINK,
  PBUF_RAW
} pbuf_layer;
typedef enum {
  PBUF_RAM,
  PBUF_ROM,
  PBUF_REF,
  PBUF_POOL
} pbuf_flag;
/* Definitions for the pbuf flag field (these are not the flags that
   are passed to pbuf_alloc()). */
#define PBUF_FLAG_RAM   0x00U    /* Flags that pbuf data is stored in RAM */
#define PBUF_FLAG_ROM   0x01U    /* Flags that pbuf data is stored in ROM */
#define PBUF_FLAG_POOL  0x02U    /* Flags that the pbuf comes from the pbuf pool */
#define PBUF_FLAG_REF   0x04U    /* Flags thet the pbuf payload refers to RAM */
/** indicates this packet was broadcast on the link */
#define PBUF_FLAG_LINK_BROADCAST 0x80U
struct pbuf {
  /** next pbuf in singly linked pbuf chain */
  struct pbuf *next;
  /** pointer to the actual data in the buffer */
  void *payload;
  /**
   * total length of this buffer and all next buffers in chain
   * belonging to the same packet.
   *
   * For non-queue packet chains this is the invariant:
   * p->tot_len == p->len + (p->next? p->next->tot_len: 0)
   */
  u16_t tot_len;
  /* length of this buffer */
  u16_t len;  
  /* flags telling the type of pbuf */
  u16_t flags;
  /**
   * the reference count always equals the number of pointers
   * that refer to this pbuf. This can be pointers from an application,
   * the stack itself, or pbuf->next pointers from a chain.
   */
  u16_t ref;
};
////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////
// #include "lwip/inet.h"		// includes ip_addr.h 	// includes arch.h 	// includes cc.h 	// includes opt.h
#include "lwip/ip_addr.h"
#include "../../../types.h"
#ifndef ip_addr_t
typedef struct ip_addr ip_addr_t;
#endif
////////////////////////////////////////////////////////////////////////////////////////////
////  FROM inet.h  /////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////
u16_t inet_chksum(void *dataptr, u16_t len);
u16_t inet_chksum_pbuf(struct pbuf *p);
u16_t inet_chksum_pseudo(struct pbuf *p, struct ip_addr *src, 
			struct ip_addr *dest, u8_t proto, u16_t proto_len);
u32_t inet_addr(const char *cp);
int inet_aton(const char *cp, struct in_addr *addr);
char *inet_ntoa(struct in_addr addr); /* returns ptr to static buffer; not reentrant! */
#ifdef htons
#undef htons
#endif /* htons */
#ifdef htonl
#undef htonl
#endif /* htonl */
#ifdef ntohs
#undef ntohs
#endif /* ntohs */
#ifdef ntohl
#undef ntohl
#endif /* ntohl */
#if BYTE_ORDER == BIG_ENDIAN
#define htons(x) (x)
#define ntohs(x) (x)
#define htonl(x) (x)
#define ntohl(x) (x)
#else
#ifdef LWIP_PREFIX_BYTEORDER_FUNCS
/* workaround for naming collisions on some platforms */
#define htons lwip_htons
#define ntohs lwip_ntohs
#define htonl lwip_htonl
#define ntohl lwip_ntohl
#endif
u16_t htons(u16_t x);
u16_t ntohs(u16_t x);
u32_t htonl(u32_t x);
u32_t ntohl(u32_t x);
#endif
////////////////////////////////////////////////////////////////////////////////////////////
//// FROM ip.h /////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////
#define IP_HLEN 20
#define IP_PROTO_ICMP 1
#define IP_PROTO_UDP 17
#define IP_PROTO_UDPLITE 170
#define IP_PROTO_TCP 6
/* This is passed as the destination address to ip_output_if (not
   to ip_output), meaning that an IP header already is constructed
   in the pbuf. This is used when TCP retransmits. */
#ifdef IP_HDRINCL
#undef IP_HDRINCL
#endif /* IP_HDRINCL */
#define IP_HDRINCL  NULL
/* This is the common part of all PCB types. It needs to be at the
   beginning of a PCB type definition. It is located here so that
   changes to this common part are made in one location instead of
   having to change all PCB structs. */
#define IP_PCB struct ip_addr local_ip; \
  struct ip_addr remote_ip; \
   /* Socket options */  \
  u16_t so_options;      \
   /* Type Of Service */ \
  u8_t tos;              \
  /* Time To Live */     \
  u8_t ttl
/*
 * Option flags per-socket. These are the same like SO_XXX.
 */
#define	SOF_DEBUG	    (u16_t)0x0001U		/* turn on debugging info recording */
#define	SOF_ACCEPTCONN	(u16_t)0x0002U		/* socket has had listen() */
#define	SOF_REUSEADDR	(u16_t)0x0004U		/* allow local address reuse */
#define	SOF_KEEPALIVE	(u16_t)0x0008U		/* keep connections alive */
#define	SOF_DONTROUTE	(u16_t)0x0010U		/* just use interface addresses */
#define	SOF_BROADCAST	(u16_t)0x0020U		/* permit sending of broadcast msgs */
#define	SOF_USELOOPBACK	(u16_t)0x0040U		/* bypass hardware when possible */
#define	SOF_LINGER	    (u16_t)0x0080U		/* linger on close if data present */
#define	SOF_OOBINLINE	(u16_t)0x0100U		/* leave received OOB data in line */
#define	SOF_REUSEPORT	(u16_t)0x0200U		/* allow local address & port reuse */
#ifdef PACK_STRUCT_USE_INCLUDES
#  include "arch/bpstruct.h"
#endif
PACK_STRUCT_BEGIN
struct ip_hdr {
  /* version / header length / type of service */
  PACK_STRUCT_FIELD(u16_t _v_hl_tos);
  /* total length */
  PACK_STRUCT_FIELD(u16_t _len);
  /* identification */
  PACK_STRUCT_FIELD(u16_t _id);
  /* fragment offset field */
  PACK_STRUCT_FIELD(u16_t _offset);
#define IP_RF 0x8000        /* reserved fragment flag */
#define IP_DF 0x4000        /* dont fragment flag */
#define IP_MF 0x2000        /* more fragments flag */
#define IP_OFFMASK 0x1fff   /* mask for fragmenting bits */
  /* time to live / protocol*/
  PACK_STRUCT_FIELD(u16_t _ttl_proto);
  /* checksum */
  PACK_STRUCT_FIELD(u16_t _chksum);
  /* source and destination IP addresses */
  PACK_STRUCT_FIELD(struct ip_addr src);
  PACK_STRUCT_FIELD(struct ip_addr dest); 
} PACK_STRUCT_STRUCT;
PACK_STRUCT_END
#ifdef PACK_STRUCT_USE_INCLUDES
#  include "arch/epstruct.h"
#endif
#define IPH_V(hdr)  (ntohs((hdr)->_v_hl_tos) >> 12)
#define IPH_HL(hdr) ((ntohs((hdr)->_v_hl_tos) >> 8) & 0x0f)
#define IPH_TOS(hdr) (htons((ntohs((hdr)->_v_hl_tos) & 0xff)))
#define IPH_LEN(hdr) ((hdr)->_len)
#define IPH_ID(hdr) ((hdr)->_id)
#define IPH_OFFSET(hdr) ((hdr)->_offset)
#define IPH_TTL(hdr) (ntohs((hdr)->_ttl_proto) >> 8)
#define IPH_PROTO(hdr) (ntohs((hdr)->_ttl_proto) & 0xff)
#define IPH_CHKSUM(hdr) ((hdr)->_chksum)
#define IPH_VHLTOS_SET(hdr, v, hl, tos) (hdr)->_v_hl_tos = (htons(((v) << 12) | ((hl) << 8) | (tos)))
#define IPH_LEN_SET(hdr, len) (hdr)->_len = (len)
#define IPH_ID_SET(hdr, id) (hdr)->_id = (id)
#define IPH_OFFSET_SET(hdr, off) (hdr)->_offset = (off)
#define IPH_TTL_SET(hdr, ttl) (hdr)->_ttl_proto = (htons(IPH_PROTO(hdr) | ((ttl) << 8)))
#define IPH_PROTO_SET(hdr, proto) (hdr)->_ttl_proto = (htons((proto) | (IPH_TTL(hdr) << 8)))
#define IPH_CHKSUM_SET(hdr, chksum) (hdr)->_chksum = (chksum)
////////////////////////////////////////////////////////////////////////////////////////////
//// FROM sockets.h ////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////
struct sockaddr_in {
  u8_t sin_len;
  u8_t sin_family;
  u16_t sin_port;
  struct in_addr sin_addr;
  char sin_zero[8];
};
struct sockaddr {
  u8_t sa_len;
  u8_t sa_family;
  char sa_data[14];
};
#ifndef socklen_t
#  define socklen_t int
#endif
#define SOCK_STREAM     1
#define SOCK_DGRAM      2
#define SOCK_RAW        3
/*
 * Option flags per-socket.
 */
#define  SO_DEBUG  0x0001    /* turn on debugging info recording */
#define  SO_ACCEPTCONN  0x0002    /* socket has had listen() */
#define  SO_REUSEADDR  0x0004    /* allow local address reuse */
#define  SO_KEEPALIVE  0x0008    /* keep connections alive */
#define  SO_DONTROUTE  0x0010    /* just use interface addresses */
#define  SO_BROADCAST  0x0020    /* permit sending of broadcast msgs */
#define  SO_USELOOPBACK  0x0040    /* bypass hardware when possible */
#define  SO_LINGER  0x0080    /* linger on close if data present */
#define  SO_OOBINLINE  0x0100    /* leave received OOB data in line */
#define	 SO_REUSEPORT	0x0200		/* allow local address & port reuse */
#define SO_DONTLINGER   (int)(~SO_LINGER)
/*
 * Additional options, not kept in so_options.
 */
#define SO_SNDBUF    0x1001    /* send buffer size */
#define SO_RCVBUF    0x1002    /* receive buffer size */
#define SO_SNDLOWAT  0x1003    /* send low-water mark */
#define SO_RCVLOWAT  0x1004    /* receive low-water mark */
#define SO_SNDTIMEO  0x1005    /* send timeout */
#define SO_RCVTIMEO  0x1006    /* receive timeout */
#define SO_ERROR     0x1007    /* get error status and clear */
#define SO_TYPE      0x1008    /* get socket type */
/*
 * Structure used for manipulating linger option.
 */
struct linger {
       int l_onoff;                /* option on/off */
       int l_linger;               /* linger time */
};
/*
 * Level number for (get/set)sockopt() to apply to socket itself.
 */
#define  SOL_SOCKET  0xfff    /* options for socket level */
#define AF_UNSPEC       0
#define AF_INET         2
#define PF_INET         AF_INET
#define PF_UNSPEC       AF_UNSPEC
#define IPPROTO_IP      0
#define IPPROTO_TCP     6
#define IPPROTO_UDP     17
#define INADDR_ANY      0
#define INADDR_BROADCAST 0xffffffff
/* Flags we can use with send and recv. */
#define MSG_DONTWAIT    0x40            /* Nonblocking i/o for this operation only */
/*
 * Options for level IPPROTO_IP
 */
#define IP_TOS       1
#define IP_TTL       2
#define IPTOS_TOS_MASK          0x1E
#define IPTOS_TOS(tos)          ((tos) & IPTOS_TOS_MASK)
#define IPTOS_LOWDELAY          0x10
#define IPTOS_THROUGHPUT        0x08
#define IPTOS_RELIABILITY       0x04
#define IPTOS_LOWCOST           0x02
#define IPTOS_MINCOST           IPTOS_LOWCOST
#ifndef FD_SET
  #undef  FD_SETSIZE
  #define FD_SETSIZE    16
  #define FD_SET(n, p)  ((p)->fd_bits[(n)/8] |=  (1 << ((n) & 7)))
  #define FD_CLR(n, p)  ((p)->fd_bits[(n)/8] &= ~(1 << ((n) & 7)))
  #define FD_ISSET(n,p) ((p)->fd_bits[(n)/8] &   (1 << ((n) & 7)))
  #define FD_ZERO(p)    memset((void*)(p),0,sizeof(*(p)))
  typedef struct fd_set {
  	unsigned char fd_bits [(FD_SETSIZE+7)/8];
  } fd_set;
  struct timeval {
    long    tv_sec;         /* seconds */
    long    tv_usec;        /* and microseconds */
  };
#endif
////////////////////////////////////////////////////////////////////////////////////////////
////  FROM icmp.h  /////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////
#define ICMP_ER 0      /* echo reply */
#define ICMP_DUR 3     /* destination unreachable */
#define ICMP_SQ 4      /* source quench */
#define ICMP_RD 5      /* redirect */
#define ICMP_ECHO 8    /* echo */
#define ICMP_TE 11     /* time exceeded */
#define ICMP_PP 12     /* parameter problem */
#define ICMP_TS 13     /* timestamp */
#define ICMP_TSR 14    /* timestamp reply */
#define ICMP_IRQ 15    /* information request */
#define ICMP_IR 16     /* information reply */
enum icmp_dur_type {
  ICMP_DUR_NET = 0,    /* net unreachable */
  ICMP_DUR_HOST = 1,   /* host unreachable */
  ICMP_DUR_PROTO = 2,  /* protocol unreachable */
  ICMP_DUR_PORT = 3,   /* port unreachable */
  ICMP_DUR_FRAG = 4,   /* fragmentation needed and DF set */
  ICMP_DUR_SR = 5      /* source route failed */
};
enum icmp_te_type {
  ICMP_TE_TTL = 0,     /* time to live exceeded in transit */
  ICMP_TE_FRAG = 1     /* fragment reassembly time exceeded */
};
PACK_STRUCT_BEGIN
struct icmp_echo_hdr {
  PACK_STRUCT_FIELD(u16_t _type_code);
  PACK_STRUCT_FIELD(u16_t chksum);
  PACK_STRUCT_FIELD(u16_t id);
  PACK_STRUCT_FIELD(u16_t seqno);
} PACK_STRUCT_STRUCT;
PACK_STRUCT_END
PACK_STRUCT_BEGIN
struct icmp_dur_hdr {
  PACK_STRUCT_FIELD(u16_t _type_code);
  PACK_STRUCT_FIELD(u16_t chksum);
  PACK_STRUCT_FIELD(u32_t unused);
} PACK_STRUCT_STRUCT;
PACK_STRUCT_END
PACK_STRUCT_BEGIN
struct icmp_te_hdr {
  PACK_STRUCT_FIELD(u16_t _type_code);
  PACK_STRUCT_FIELD(u16_t chksum);
  PACK_STRUCT_FIELD(u32_t unused);
} PACK_STRUCT_STRUCT;
PACK_STRUCT_END
#define ICMPH_TYPE(hdr) (ntohs((hdr)->_type_code) >> 8)
#define ICMPH_CODE(hdr) (ntohs((hdr)->_type_code) & 0xff)
#define ICMPH_TYPE_SET(hdr, type) ((hdr)->_type_code = htons(ICMPH_CODE(hdr) | ((type) << 8)))
#define ICMPH_CODE_SET(hdr, code) ((hdr)->_type_code = htons((code) | (ICMPH_TYPE(hdr) << 8)))
////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////
#endif /* __XV6CHAT_NETTYPES_H__ */