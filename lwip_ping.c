/* lwip_ping.c */

// #include "lwip/opt.h"
#include "lwip_ping.h"
#include "types.h"
#include "defs.h"
#include "stdio.h"
// #include "string.h"
#include "thread.h"

#if LWIP_RAW /* don't build if not configured for use in lwipopts.h */

#include "lwip/mem.h"
#include "lwip/raw.h"
#include "lwip/icmp.h"
#include "lwip/netif.h"
#include "lwip/sys.h"
#include "lwip/tcp.h"
// #include "lwip/timers.h"
// #include "lwip/inet_chksum.h"

#if PING_USE_SOCKETS
#include "lwip/sockets.h"
// #include "lwip/inet.h"
#endif /* PING_USE_SOCKETS */


/**
 * PING_DEBUG: Enable debugging for PING.
 */
#ifndef PING_DEBUG
#define PING_DEBUG     ICMP_DEBUG
#endif

/** ping target - should be a "ip_addr_t" */
#ifndef PING_TARGET_DEFAULT
#define PING_TARGET_DEFAULT   (netif_default?netif_default->gw:ip_addr_any)
#endif

/** ping receive timeout - in milliseconds */
#ifndef PING_RCV_TIMEO
#define PING_RCV_TIMEO 1500 //1200 //1000
#endif

/** ping delay - in milliseconds */
#ifndef PING_DELAY
#define PING_DELAY     3000 //2500 //1000
#endif

/** ping identifier - must fit on a u16_t */
#ifndef PING_ID
#define PING_ID        0xAFAF
#endif

/** ping additional data size to include in the packet */
#ifndef PING_DATA_SIZE
#define PING_DATA_SIZE 32
#endif

/** ping result action - no default action */
#ifndef PING_RESULT
#define PING_RESULT(ping_ok)  (cprintf("\t[ping result: %d]\n", ((int)(ping_ok))))
#endif

/* ping variables */
static u16_t ping_seq_num;
static u32_t start_time;
static u32_t ping_sent_time;
static u32_t ping_time;
static ip_addr_t ping_addr;

int global_s;
int ping_cnt;
int last_ping_good;
#define THREAD_PING_COUNT 5

#if !PING_USE_SOCKETS
static struct raw_pcb *ping_pcb;
#endif /* PING_USE_SOCKETS */

static void 
ip_addr_print(ip_addr_t *ipaddr)
{
  cprintf("%u.%u.%u.%u",
            ip4_addr1(ipaddr),
            ip4_addr2(ipaddr),
            ip4_addr3(ipaddr),
            ip4_addr4(ipaddr));
}

static void
time_to_str(char retstr[], int n, u32_t t)
{
  int ndigits = 0;
  int temp = t;
  while (temp) {
    temp /= 10;
    ++ndigits;
  }
  assert(ndigits <= n);
  char ptimestr[ndigits];
  snprintf(ptimestr, ndigits, "%u", t);
  cprintf("%u vs. %s\n", t, ptimestr);

  if (ndigits == 1)
    snprintf(retstr, ndigits+1, "0.%c0", ptimestr[0]);
  else if (ndigits == 2)
    snprintf(retstr, ndigits+1, "%c.%c0", ptimestr[0], ptimestr[1]);
  else if (ndigits == 3)
    snprintf(retstr, ndigits+1, "%c%c.%c0", ptimestr[0], ptimestr[1], ptimestr[2]);
  else if (ndigits == 4)
    snprintf(retstr, ndigits+1, "%c%c%c.%c0", ptimestr[0], ptimestr[1], ptimestr[2], ptimestr[3]);
  else if (ndigits == 5)
    snprintf(retstr, ndigits+1, "%c%c%c%c.%c0", ptimestr[0], ptimestr[1], ptimestr[2], ptimestr[3], ptimestr[4]);
  else {
    cprintf("[time_to_str] OVERFLOW\n");
    // return ptimestr;
  }
  // return retstr;
}

static void
print_result(int ping_ok, int psize, u32_t ptime)
{
  // static int icmp_seq = 0;
  // cprintf("ptime:%u\n", ptime);

  if (ping_ok > 0) 
  {
    // char ptimestr[6];
    // time_to_str(ptimestr, 6, ptime);
    // cprintf("%d bytes from %u.%u.%u.%u: icmp_seq=%u ttl=%d time=%s ms\n", psize, 
    //     ip4_addr1(&ping_addr), ip4_addr2(&ping_addr), ip4_addr3(&ping_addr), ip4_addr4(&ping_addr), 
    //     ping_seq_num, ICMP_TTL, ptimestr);
    cprintf("%d bytes from %u.%u.%u.%u: icmp_seq=%u ttl=%d time=%u ms\n", psize, 
        ip4_addr1(&ping_addr), ip4_addr2(&ping_addr), ip4_addr3(&ping_addr), ip4_addr4(&ping_addr), 
        ping_seq_num-1, ICMP_TTL, ptime/10);
  } else if (!ping_ok) {
    cprintf("Request timeout for icmp_seq %u\n", ping_seq_num-1);
  } else {
    cprintf("ERROR: ping dropped\n");
  }
  // icmp_seq++;
}

/** Prepare a echo ICMP request */
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


/* Ping using the socket ip */
static err_t
ping_send(int s, ip_addr_t *addr)
{
  int err;
  struct icmp_echo_hdr *iecho;
  struct sockaddr_in to;
  size_t ping_size = sizeof(struct icmp_echo_hdr) + PING_DATA_SIZE;
  LWIP_ASSERT("ping_size is too big", ping_size <= 0xffff);

  iecho = (struct icmp_echo_hdr *)mem_malloc((mem_size_t)ping_size);
  if (!iecho) {
    return ERR_MEM;
  }

  ping_prepare_echo(iecho, (u16_t)ping_size);

  to.sin_len = sizeof(to);
  to.sin_family = AF_INET;
  // inet_addr_from_ipaddr(&to.sin_addr, addr);
  to.sin_addr.s_addr = addr->addr;

  ping_sent_time = millitime();
  err = lwip_sendto(s, iecho, ping_size, 0, (struct sockaddr*)&to, sizeof(to));

  mem_free(iecho);

  return (err ? ERR_OK : ERR_VAL);
}

static void
ping_recv(int s)
{
  char buf[64];
  int fromlen, len;
  struct sockaddr_in from;
  struct ip_hdr *iphdr;
  struct icmp_echo_hdr *iecho;

  u32_t rcv_start_time=millitime();
  u32_t rcv_attempt_time=rcv_start_time;

  // tcp_tmr();

  // len = lwip_recvfrom(s, buf, sizeof(buf), 0, (struct sockaddr*)&from, (socklen_t*)&fromlen);
  // cprintf("**** [ping_recv] len=%d\n", len);
  // last_ping_good = 0;
  while(((len = lwip_recvfrom(s, buf, sizeof(buf), 0, (struct sockaddr*)&from, (socklen_t*)&fromlen)) > 0))
      // || (rcv_attempt_time-rcv_start_time < 45000))
  {
    LWIP_DEBUGF(PING_DEBUG,("[ping_recv] len=%d\n", len));
    ping_time = millitime() - ping_sent_time;
    if (len >= (int)(sizeof(struct ip_hdr)+sizeof(struct icmp_echo_hdr))) 
    {
      last_ping_good = 1;

      ip_addr_t fromaddr;
      fromaddr.addr = from.sin_addr.s_addr;

      LWIP_DEBUGF(PING_DEBUG, ("ping: recv "));
      ip_addr_debug_print(PING_DEBUG, &fromaddr);
      // ip_addr_print(&fromaddr);
      LWIP_DEBUGF(PING_DEBUG, (" %u ms\n", ping_time));
      // LWIP_DEBUGF2(PING_DEBUG, (" %u ms\n"), (millitime() - ping_time));

      iphdr = (struct ip_hdr *)buf;
      iecho = (struct icmp_echo_hdr *)(buf + (IPH_HL(iphdr) * 4));

      // rcv_attempt_time = millitime();

      if ((iecho->id == PING_ID) && (iecho->seqno == htons(ping_seq_num))) 
      {
        last_ping_good = 1;
        /* do some ping result processing */
        // PING_RESULT((ICMPH_TYPE(iecho) == ICMP_ER));
        print_result((int)(ICMPH_TYPE(iecho) == ICMP_ER), len, ping_time); //rcv_attempt_time));
        return;
      } 
      else
        LWIP_DEBUGF(PING_DEBUG, ("ping: drop\n"));
        // cprintf("ping: drop\n");
        // print_result(0, 0, (millitime()-ping_time)); //rcv_attempt_time));
    }
    // else
      rcv_attempt_time = millitime();
  }

  if (len == 0) {
    last_ping_good = 0;
    ping_time = millitime() - ping_sent_time;
    // LWIP_DEBUGF(PING_DEBUG, ("ping: recv - %u ms - timeout\n", (millitime()-ping_time)));
    // cprintf("ping: recv - %u ms - timeout\n", (millitime()-ping_time));
    print_result(0, 0, ping_time); //rcv_attempt_time));
  }

  /* do some ping result processing */
  // PING_RESULT(0);
  print_result(-1, 0, 0);
}



static void
ping_timeout(void *arg)
{
  // tcp_tmr();
  // cprintf("[ping_timeout]\n");
  // ping_seq_num = 0;
  // kptable_dump();
  // tcp_timer_needed();
  // sys_timeout(PING_DELAY/25, ping_timeout, 0);
  // LWIP_DEBUGF(PING_DEBUG,("[ping_timeout]\n"));
  if (ping_cnt+1 <= THREAD_PING_COUNT) {
    if (!last_ping_good)
      print_result(0, 0, ping_time);
    ping_cnt++;
    ping_seq_num++;
    sys_timeout(PING_RCV_TIMEO, ping_timeout, 0);
    // sys_timeout(PING_DELAY, ping_timeout, 0);
    // ping_send(global_s, &ping_addr);
    // ping_recv(global_s);
  }
  else {
    // LWIP_DEBUGF(PING_DEBUG,("PING THREAD EXITING\n"));
    if (!last_ping_good)
      cprintf("Destination host unreachable.\n");
    lwip_close(global_s);
    ping_seq_num = ping_cnt = last_ping_good = 0;
    exit();
  }
  // ping_send(global_s, &ping_addr);
  // sys_timeout(PING_DELAY, ping_timeout, 0);
  // sys_timeout(0xffffffff, ping_timeout, 0);
   // exit();
}


static void
ping_thread(void *arg)
{
  int s; //, cnt=0;
  int timeout = PING_RCV_TIMEO;
  ip_addr_t ping_target;

  // tcp_tmr();
  // Add a sys_timeout structure for ping thread into the lwip_system_timeouts list
  // sys_timeout(PING_DELAY/25, ping_timeout, 0);
  // sys_timeout((int)(PING_DELAY*1.2), ping_timeout, 0); 
  sys_timeout(PING_DELAY, ping_timeout, 0); 
  // sys_timeout(PING_RCV_TIMEO*3, ping_timeout, 0);
  // sys_timeout(1000, ping_timeout, 0); 
  // sys_timeout(0xffffffff, ping_timeout, 0);

  cprintf("PING ");
  ip_addr_print(&ping_addr);
  cprintf(": %d data bytes\n", (int)(sizeof(struct icmp_echo_hdr) + PING_DATA_SIZE));
  timed_sleep(15);
  // int i=0;
  // while(++i < 150000);  // Hacky delay

  if ((s = lwip_socket(AF_INET, SOCK_RAW, IP_PROTO_ICMP)) < 0) {
    cprintf("ERROR from lwip_socket!\n");
    ping_seq_num = 0;
    exit();
  }
  global_s = s;

  lwip_setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));

  start_time = millitime();

  while (ping_cnt++ < THREAD_PING_COUNT) 
  {
    // if (++cnt > THREAD_PING_COUNT)
    //   break;

    // ping_target = PING_TARGET_DEFAULT;
    ping_target = ping_addr;

    if (ping_send(s, &ping_target) == ERR_OK) 
    {
      LWIP_DEBUGF(PING_DEBUG, ("ping: send "));
      ip_addr_debug_print(PING_DEBUG, &ping_target);
      // ip_addr_print(&ping_target);
      LWIP_DEBUGF(PING_DEBUG, ("\n"));
      // cprintf("ping sent!\n");

      // ping_time = millitime() - start_time;
      // LWIP_DEBUGF(PING_DEBUG,("ping_time: %ums\n", ping_time));
      
      ping_recv(s);
      // LWIP_DEBUGF(PING_DEBUG,("ping_recv returned!\n"));
    } 
    else 
    {
      LWIP_DEBUGF(PING_DEBUG, ("ping: send "));
      ip_addr_debug_print(PING_DEBUG, &ping_target);
      // ip_addr_print(&ping_target);
      LWIP_DEBUGF(PING_DEBUG, (" - error\n"));
      cprintf("ERROR from ping_send!\n");
    }
    // sys_msleep(PING_DELAY);
    // int i=0;
    // while(++i < 10000000);  // Hacky delay
    // timed_sleep(50);
    // timed_sleep(20);
  }
  
  // cprintf("PING THREAD EXITING\n");
  LWIP_DEBUGF(PING_DEBUG,("PING THREAD EXITING\n"));
  lwip_close(global_s);
  ping_seq_num = ping_cnt = last_ping_good = 0;
  exit();
}


void
// ping_init(void)
ping_init(ip_addr_t *ping_ip)
{
  // Set the global
  // ping_addr = ping_ip != 0 ? *ping_ip : PING_TARGET_DEFAULT;
  ip_addr_t if_null;
  if (ping_ip == 0)
  {
    // if_null.addr = inet_addr("10.0.3.2");
    if_null.addr = inet_addr("127.0.0.1");
    ping_ip = &if_null;
  }
  ping_addr = *ping_ip;

  start_time = millitime();

  sys_thread_new(ping_thread, 0, DEFAULT_THREAD_PRIO, "ping thread");

    // ping_thread((void*)0);
}

#endif /* LWIP_RAW */
