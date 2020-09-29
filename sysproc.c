#include "types.h"
#include "x86.h"
#include "defs.h"
#include "date.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"
// #include "lwip/include/lwip/sockets.h"
#include "lwip/sockets.h"
// #include "lwip/ip_addr.h"
// #include "lwip/inet.h"
#include "lwip_ping.h"

int
sys_fork(void)
{
  return fork();
}

int
sys_exit(void)
{
  exit();
  return 0;  // not reached
}

int
sys_wait(void)
{
  return wait();
}

int
sys_kill(void)
{
  int pid;

  if(argint(0, &pid) < 0)
    return -1;
  return kill(pid);
}

int
sys_getpid(void)
{
  return proc->pid;
}

int
sys_sbrk(void)
{
  int addr;
  int n;

  if(argint(0, &n) < 0)
    return -1;
  addr = proc->sz;
  if(growproc(n) < 0)
    return -1;
  return addr;
}

int
timed_sleep(int n)
{
  uint ticks0;

  acquire(&tickslock);
  ticks0 = ticks;
  while(ticks - ticks0 < n){
    if(proc->killed){
      release(&tickslock);
      return -1;
    }
    sleep(&ticks, &tickslock);
  }
  release(&tickslock);
  return 0;
}

int
sys_sleep(void)
{
  int n;

  if(argint(0, &n) < 0)
    return -1;
  
  return timed_sleep(n);
}

// return how many clock tick interrupts have occurred
// since start.
int
sys_uptime(void)
{
  uint xticks;

  acquire(&tickslock);
  xticks = ticks;
  release(&tickslock);
  return xticks;
}

int 
sys_umsec(void)
{
  return millitime();
}

int
sys_pkill(void)
{
    char *pname;

    if ((argptr(0, &pname, sizeof(pname))) < 0)
        return -1;

    return pkill(pname);
}

////  LWIP SYSTEM CALL IMPLEMENTATIONS  ////
int 
sys_accept(void)
{
    int s;
    struct sockaddr *addr;
    socklen_t *addrlen;
    if((argint(0, &s)<0) || 
            (argptr(1, (char **)&addr, sizeof(struct sockaddr))<0) ||
            (argptr(2, (char **)&addrlen, sizeof(socklen_t))<0))
        return -1;
    return lwip_accept(s, addr, addrlen);
}

int 
sys_bind(void)
{
    int s;
    struct sockaddr *name;
    socklen_t namelen;
    if((argint(0, &s)<0) || 
            (argptr(1, (char **)&name, sizeof(struct sockaddr))<0) ||
            (argptr(2, (char **)&namelen, sizeof(socklen_t))<0))
        return -1;
    return lwip_bind(s, name, namelen);
}

int 
sys_shutdown(void)
{
    int s;
    int how;
    if ((argint(0, &s)<0) || (argint(1, &how) < 0))
        return -1;
    return lwip_shutdown(s, how);
}

int 
sys_getsockopt(void)
{
    int s;
    int level;
    int optname;
    void *optval;
    socklen_t *optlen;
    if ((argint(0, &s)<0) ||
        (argint(1, &level)<0) ||
        (argint(2, &optname)<0) ||
        (argptr(4, (char **)&optlen, sizeof(socklen_t))<0) ||
        (argptr(3, (char **)&optval, 0)<0))
        return -1;
    return lwip_getsockopt(s, level, optname, optval, optlen);
}

int sys_setsockopt(void)
{
    int s;
    int level;
    int optname;
    void *optval;
    socklen_t *optlen;
    if ((argint(0, &s)<0) ||
        (argint(1, &level)<0) ||
        (argint(2, &optname)<0) ||
        (argptr(4, (char **)&optlen, sizeof(socklen_t))<0) ||
        (argptr(3, (char **)&optval, *optlen)<0))
        return -1;
    return lwip_setsockopt(s, level, optname, optval, *optlen);
}

int sys_sockclose(void)
{
    int s;
    if (argint(0, &s) <0)
        return -1;
    return lwip_close(s);
}

int sys_connect(void)
{
    int s;
    struct sockaddr *name;
    socklen_t namelen;
    if((argint(0, &s)<0) || 
            (argptr(1, (char **)&name, sizeof(struct sockaddr))<0) ||
            (argint(2, &namelen)<0))
        return -1;
    return lwip_connect(s, name, namelen);
}

int sys_listen(void)
{
    int s;
    int backlog;
    if ((argint(0, &s)<0) ||
        (argint(1, &backlog)<0))
        return -1;
    return lwip_listen(s, backlog);
}

int sys_recv(void)
{
    int s;
    void *mem;
    int len;
    unsigned int flags;
    if ((argint(0, &s)<0) ||
        (argint(2, &len)<0) ||
        (argptr(1, (char **)&mem, len)<0) ||
        (argint(3, (int *)&flags)<0))
        return -1;
    return lwip_recv(s, mem, len, flags);
}

int sys_recvfrom(void)
{
    int s;
    void *mem;
    int len;
    unsigned int flags;
    struct sockaddr *from;
    socklen_t *fromlen;
    if ((argint(0, &s)<0) ||
        (argint(2, &len)<0) ||
        (argptr(1, (char **)&mem, len)<0) ||
        (argint(3, (int *)&flags)<0) ||
        (argptr(4, (char **)&from, 0)<0) ||
        (argptr(5, (char **)&fromlen, sizeof(socklen_t))<0))
        return -1;
    return lwip_recvfrom(s, mem, len, flags, from, fromlen);
}

int sys_send(void)
{
    int s;
    void *dataptr;
    int size;
    unsigned int flags;
    if ((argint(0, &s)<0) ||
        (argint(2, &size)<0) ||
        (argptr(1, (char **)&dataptr, size)<0) ||
        (argint(3, (int *)&flags)<0))
        return -1;
    memmove(P2V_WO(dataptr), dataptr, size);
    return lwip_send(s, P2V_WO(dataptr), size, flags);
}

int sys_sendto(void)
{
    int s;
    void *dataptr;
    int size;
    unsigned int flags;
    struct sockaddr *to;
    socklen_t *tolen;
    if ((argint(0, &s)<0) ||
        (argint(2, &size)<0) ||
        (argptr(1, (char **)&dataptr, size)<0) ||
        (argint(3, (int *)&flags)<0) ||
        (argptr(5, (char **)&tolen, sizeof(socklen_t))<0) ||
        (argptr(4, (char **)&to, *tolen)<0))
        return -1;
    return lwip_sendto(s, P2V_WO(dataptr), size, flags, to, *tolen);
}

int sys_socket(void)
{
    int domain;
    int type;
    int protocol;
    if ((argint(0, &domain)<0) ||
        (argint(1, &type)<0) ||
        (argint(2, &protocol)<0))
        return -1;
    return lwip_socket(domain, type, protocol);
}

int
sys_getpeername(void)
{
    int s;
    struct sockaddr *name;
    socklen_t *namelen;
    if((argint(0, &s)<0) || 
            (argptr(1, (char **)&name, sizeof(struct sockaddr))<0) ||
            (argptr(2, (char **)&namelen, sizeof(socklen_t))<0))
        return -1;
    return lwip_getpeername(s, name, namelen);
}

int
sys_getsockname(void)
{
    int s;
    struct sockaddr *name;
    socklen_t *namelen;
    if((argint(0, &s)<0) || 
            (argptr(1, (char **)&name, sizeof(struct sockaddr))<0) ||
            (argptr(2, (char **)&namelen, sizeof(socklen_t))<0))
        return -1;
    return lwip_getsockname(s, name, namelen);
}

int 
sys_select(void)
{
    int maxfdp1;
    fd_set *readset, *writeset, *exceptset;
    struct timeval *timeout;
    if ((argint(0, &maxfdp1)< 0) ||
            (argptr(1, (char **)&readset, sizeof(fd_set))<0) ||
            (argptr(2, (char **)&writeset, sizeof(fd_set))<0) ||
            (argptr(3, (char **)&exceptset, sizeof(fd_set))<0) ||
            (argptr(4, (char **)&timeout, sizeof(struct timeval))<0))
        return -1;
    return lwip_select(maxfdp1, readset, writeset, exceptset, timeout);
}

int 
sys_ioctl(void)
{
    int s; 
    long cmd; 
    void **argp;
    if ((argint(0, &s)< 0) ||
            (argint(1, (int *)&cmd)<0) ||
            (argptr(2, (char **)&argp, sizeof(void*))<0))
        return -1;
    assert(argp);
    return lwip_ioctl(s, cmd, *argp);
}

int 
sys_send_ping(void)
{
    char *addr_str;
    // struct ip_addr ping_addr;
    ip_addr_t ping_addr;

    if (argstr(0, &addr_str) < 0)
        return -1;

    ping_addr.addr = inet_addr(addr_str);

    // cprintf("[sys_send_ping #1] ping_addr: ");
    // ip_addr_debug_print(ICMP_DEBUG, &ping_addr);
    // cprintf("\n");
    // cprintf("[sys_send_ping #2] ping_addr: %u.%u.%u.%u\n",
    //         ip4_addr1(&ping_addr),
    //         ip4_addr2(&ping_addr),
    //         ip4_addr3(&ping_addr),
    //         ip4_addr4(&ping_addr));

    ping_init(&ping_addr);
    return 0;
}


extern struct proc* ptable_find_proc(char *tofind);
int 
sys_find_proc(void)
{
    char* procname;
    if (argstr(0, &procname) < 0)
        return 0; //-1;
    return (int)ptable_find_proc(procname);
}
