/* User-level ping implementation */

#include "user.h"
#include "lwip/sockets.h"
#include "lwip/inet.h"
// #include "lwip/nettypes.h"
#include "lwip/err.h"
#include "lwip/icmp.h"
#include "lwip/ip.h"
#include "types.h"

#ifndef ip_addr_t 
#define ip_addr_t struct ip_addr 
#endif

#define PING_SEND_COUNT 5

/** ping receive timeout - in milliseconds */
#ifndef PING_RCV_TIMEO
#define PING_RCV_TIMEO 1000
#endif

/** ping delay - in milliseconds */
#ifndef PING_DELAY
#define PING_DELAY     2500 //1000
#endif

/** ping identifier - must fit on a u16_t */
#ifndef PING_ID
#define PING_ID        0xAFAF
#endif

/** ping additional data size to include in the packet */
#ifndef PING_DATA_SIZE
#define PING_DATA_SIZE 32
#endif

/* ping variables */
static u16_t ping_seq_num;
static int start_time;
static int ping_sent_time;
static int ping_time;
static ip_addr_t ping_addr;
static int global_s;
static char *ping_addr_str;



// extern int lwip_setsockopt (int s, int level, int optname, const void *optval, socklen_t optlen);
// extern int lwip_close(int s);
// extern int lwip_recv(int s, void *mem, int len, unsigned int flags);
// extern int lwip_recvfrom(int s, void *mem, int len, unsigned int flags,
//       struct sockaddr *from, socklen_t *fromlen);
// extern int lwip_send(int s, void *dataptr, int size, unsigned int flags);
// extern int lwip_sendto(int s, void *dataptr, int size, unsigned int flags,
//     struct sockaddr *to, socklen_t tolen);
// extern int lwip_socket(int domain, int type, int protocol);



static void 
ip_addr_print(ip_addr_t *ipaddr)
{
    printf(1, "%u.%u.%u.%u",
        ip4_addr1(ipaddr),
        ip4_addr2(ipaddr),
        ip4_addr3(ipaddr),
        ip4_addr4(ipaddr));
}

static void
ping_exit(void)
{
    sockclose(global_s);
    // lwip_close(global_s);
    ping_seq_num = 0;
    printf(1, "--- %s ping finished ---\n", ping_addr_str);
    exit();
}

static void
print_result(int ping_ok, int psize, u32_t ptime)
{
    if (ping_ok > 0) 
    {
        printf(1, "%d bytes from %u.%u.%u.%u: icmp_seq=%u ttl=%d time=%d ms\n", 
                psize, 
                ip4_addr1(&ping_addr), ip4_addr2(&ping_addr), 
                ip4_addr3(&ping_addr), ip4_addr4(&ping_addr), 
                ping_seq_num-1, ICMP_TTL, ptime/10);
    } 
    else if (!ping_ok)  // recevied packet length was 0
        printf(1, "Request timeout for icmp_seq %u\n", ping_seq_num-1);
    // else
    //  printf(2, "ERROR: ping dropped\n");
}

static void
process_setsockopt_err(int err)
{
    switch(err) {
        case (-1):
            printf(2, "\tlwip_setsockopt err: GENERIC #1\n");
            break;
        case (-2):
            printf(2, "\tlwip_setsockopt err: get_socket RETURNED NULL\n");
            break;
        case (-3):
            printf(2, "\tlwip_setsockopt err: OPTVAL (ARG 3) WAS NULL\n");
            break;
        case (-4):
            printf(2, "\tlwip_setsockopt err: GENERIC #2\n");
            break;
        default:
            break;
    }
}

static void
process_sendto_err(int err)
{
    switch(err) {
        case (-13):
            printf(2, "\tlwip_sendto err: get_socket RETURNED NULL\n");
            break;
        case (-14):
            printf(2, "\tlwip_send err: get_socket RETURNED NULL\n");
            break;
        case (-15):
            printf(2, "\tlwip_send err: netbuf_new() RETURNED NULL\n");
            break;
        case (ERR_MEM):
            printf(2, "\tlwip_send/netconn_send err: ERR_MEM -- memp_malloc() FAILED\n");
            break;
        case (ERR_VAL):
            printf(2, "\tlwip_send/netconn_send err: ERR_VAL -- sock->conn == NULL\n");
            break;
        default:
            printf(2, "\tprocess_sendto_err passed an unimplemented error code: %d\n", err);
            break;
    }
}

static void
ping_prepare_echo(struct icmp_echo_hdr *iecho, u16_t len)
{
    size_t i;
    size_t data_len = len - sizeof(struct icmp_echo_hdr);

    ICMPH_TYPE_SET(iecho, ICMP_ECHO);
    ICMPH_CODE_SET(iecho, 0);
    iecho->chksum = 0;
    iecho->id     = PING_ID;
    iecho->seqno  = htons(++ping_seq_num);

    /* fill the additional data buffer with some data */
    for (i = 0; i < data_len; i++) {
        ((char*)iecho)[sizeof(struct icmp_echo_hdr) + i] = (char)i;
    }

    iecho->chksum = inet_chksum(iecho, len);
}

static err_t 
ping_send(int s, ip_addr_t *addr)
{
    int err;
    struct icmp_echo_hdr *iecho;
    struct sockaddr_in to;

    size_t ping_size = sizeof(struct icmp_echo_hdr) + PING_DATA_SIZE;
    if (ping_size > 0xffff)
        return ERR_VAL;

    iecho = (struct icmp_echo_hdr *)malloc(ping_size);
    if (!iecho)
        return ERR_MEM;

    ping_prepare_echo(iecho, (u16_t)ping_size);

    to.sin_len = sizeof(to);
    to.sin_family = AF_INET;
    to.sin_addr.s_addr = addr->addr;

    ping_sent_time = umsec();
    err = sendto(s, iecho, ping_size, 0, (struct sockaddr*)&to, sizeof(to));
    // err = lwip_sendto(s, iecho, ping_size, 0, (struct sockaddr*)&to, sizeof(to));
    // if (err == -1) {
    //  printf(2, "ERROR from lwip_sendto!\n");
    if (err != ping_size) {
        process_sendto_err(err);
    }


    free(iecho);

    return (err ? ERR_OK : ERR_VAL);    // ERR_OK = 0, ERR_VAL = -7
}

static void
ping_recv(int s)
{
    char buf[64];
    int fromlen, len;
    struct sockaddr_in from;
    struct ip_hdr *iphdr;
    struct icmp_echo_hdr *iecho;

    int rcv_start_time = umsec();
    int rcv_attempt_time = rcv_start_time;

    while (
            ((len = recvfrom(s, buf, sizeof(buf), 0, (struct sockaddr*)&from, (socklen_t*)&fromlen)) > 0)
        // ((len = lwip_recvfrom(s, buf, sizeof(buf), 0, (struct sockaddr*)&from, (socklen_t*)&fromlen)) > 0)
            &&
            (rcv_attempt_time-rcv_start_time < 2*PING_RCV_TIMEO))
    {
        ping_time = umsec() - ping_sent_time;

        if (len >= (int)(sizeof(struct ip_hdr)+sizeof(struct icmp_echo_hdr))) 
        {
            ip_addr_t fromaddr;
            fromaddr.addr = from.sin_addr.s_addr;

            iphdr = (struct ip_hdr *)buf;
            iecho = (struct icmp_echo_hdr *)(buf + (IPH_HL(iphdr) * 4));

            if ((iecho->id == PING_ID) && (iecho->seqno == htons(ping_seq_num))) 
            {
              /* do some ping result processing */
                print_result((int)(ICMPH_TYPE(iecho) == ICMP_ER), len, ping_time); 
                return;
            } 
            // else
            //     LWIP_DEBUGF(PING_DEBUG, ("ping: drop\n"));
            //     // cprintf("ping: drop\n");
                // print_result(0, 0, (millitime()-ping_time)); //rcv_attempt_time));
        }
        rcv_attempt_time = umsec();
    }

    if (len == 0) {
        ping_time = umsec() - ping_sent_time;
        print_result(0, 0, ping_time);  // Will report a 'Request timeout'
    }

    /* Report back that ping was dropped */
    // print_result(-1, 0, 0);
}

// static int 
// doublecheck_addr(char *addr_str)
// {
//  int dot_cnt = 0;
//  char *cp = &addr_str[0];
// }

int 
main(int argc, char *argv[])
{
    if (argc != 2) {
        printf(2, "Format Error:  ping <IP Address>\n");
        exit();
    }
    exit();
    return 0; 
} 

