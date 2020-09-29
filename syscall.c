#include "types.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"
#include "x86.h"
#include "syscall.h"

// #define LWIP_SYSCALL_DEBUG


// User code makes a system call with INT T_SYSCALL.
// System call number in %eax.
// Arguments on the stack, from the user call to the C
// library system call function. The saved user %esp points
// to a saved program counter, and then the first argument.

// Fetch the int at addr from the current process.
int
fetchint(uint addr, int *ip)
{
  if(addr >= proc->sz || addr+4 > proc->sz)
    return -1;
  *ip = *(int*)(addr);
  return 0;
}

// Fetch the nul-terminated string at addr from the current process.
// Doesn't actually copy the string - just sets *pp to point at it.
// Returns length of string, not including nul.
int
fetchstr(uint addr, char **pp)
{
  char *s, *ep;

  if(addr >= proc->sz)
    return -1;
  *pp = (char*)addr;
  ep = (char*)proc->sz;
  for(s = *pp; s < ep; s++)
    if(*s == 0)
      return s - *pp;
  return -1;
}

// Fetch the nth 32-bit system call argument.
int
argint(int n, int *ip)
{
  return fetchint(proc->tf->esp + 4 + 4*n, ip);
}

// Fetch the nth word-sized system call argument as a pointer
// to a block of memory of size n bytes.  Check that the pointer
// lies within the process address space.
int
argptr(int n, char **pp, int size)
{
  int i;

  if(argint(n, &i) < 0)
    return -1;
  if((uint)i >= proc->sz || (uint)i+size > proc->sz)
    return -1;
  *pp = (char*)i;
  return 0;
}

// Fetch the nth word-sized system call argument as a string pointer.
// Check that the pointer is valid and the string is nul-terminated.
// (There is no shared writable memory, so the string can't change
// between this check and being used by the kernel.)
int
argstr(int n, char **pp)
{
  int addr;
  if(argint(n, &addr) < 0)
    return -1;
  return fetchstr(addr, pp);
}

extern int sys_chdir(void);
extern int sys_close(void);
extern int sys_dup(void);
extern int sys_exec(void);
extern int sys_exit(void);
extern int sys_fork(void);
extern int sys_fstat(void);
extern int sys_getpid(void);
extern int sys_kill(void);
extern int sys_link(void);
extern int sys_mkdir(void);
extern int sys_mknod(void);
extern int sys_open(void);
extern int sys_pipe(void);
extern int sys_read(void);
extern int sys_sbrk(void);
extern int sys_sleep(void);
extern int sys_unlink(void);
extern int sys_wait(void);
extern int sys_write(void);
extern int sys_uptime(void);

extern int sys_arp(void);
extern int sys_arpserv(void);
extern int sys_arp_receive(void);

extern int sys_umsec(void);
extern int sys_getc(void);
extern int sys_pkill(void);
extern int sys_send_ping(void);
extern int sys_find_proc(void);

// LWIP/BSD sockets
extern int sys_accept(void);
extern int sys_bind(void);
extern int sys_shutdown(void);
extern int sys_getsockopt(void);
extern int sys_setsockopt(void);
extern int sys_sockclose(void);
extern int sys_connect(void);
extern int sys_listen(void);
extern int sys_recv(void);
extern int sys_recvfrom(void);
extern int sys_send(void);
extern int sys_sendto(void);
extern int sys_socket(void);
extern int sys_getpeername(void);
extern int sys_getsockname(void);
extern int sys_select(void);
extern int sys_ioctl(void);

static int (*syscalls[])(void) = {
[SYS_fork]    sys_fork,
[SYS_exit]    sys_exit,
[SYS_wait]    sys_wait,
[SYS_pipe]    sys_pipe,
[SYS_read]    sys_read,
[SYS_kill]    sys_kill,
[SYS_exec]    sys_exec,
[SYS_fstat]   sys_fstat,
[SYS_chdir]   sys_chdir,
[SYS_dup]     sys_dup,
[SYS_getpid]  sys_getpid,
[SYS_sbrk]    sys_sbrk,
[SYS_sleep]   sys_sleep,
[SYS_uptime]  sys_uptime,
[SYS_open]    sys_open,
[SYS_write]   sys_write,
[SYS_mknod]   sys_mknod,
[SYS_unlink]  sys_unlink,
[SYS_link]    sys_link,
[SYS_mkdir]   sys_mkdir,
[SYS_close]   sys_close,
[SYS_arp]     sys_arp,
[SYS_arpserv] sys_arpserv,
[SYS_arp_receive] sys_arp_receive,
[SYS_umsec] sys_umsec,
[SYS_getc] sys_getc,
[SYS_pkill] sys_pkill,
[SYS_send_ping] sys_send_ping,
[SYS_find_proc] sys_find_proc,
// LWIP sockets
[SYS_socket]  sys_socket,
[SYS_bind]    sys_bind,
[SYS_listen]  sys_listen,
[SYS_accept]  sys_accept,
[SYS_recv]    sys_recv,
[SYS_recvfrom] sys_recvfrom,
[SYS_send]    sys_send,
[SYS_sendto]  sys_sendto,
[SYS_shutdown] sys_shutdown,
[SYS_getsockopt] sys_getsockopt,
[SYS_setsockopt] sys_setsockopt,
[SYS_sockclose] sys_sockclose,
[SYS_connect] sys_connect,
[SYS_getpeername] sys_getpeername,
[SYS_getsockname] sys_getsockname,
[SYS_select]  sys_select,
[SYS_ioctl]   sys_ioctl,
};

static char *lwip_syscall_names[] = {
[SYS_socket]  "sys_socket",
[SYS_bind]    "sys_bind",
[SYS_listen]  "sys_listen",
[SYS_accept]  "sys_accept",
[SYS_recv]    "sys_recv",
[SYS_recvfrom] "sys_recvfrom",
[SYS_send]    "sys_send",
[SYS_sendto]  "sys_sendto",
[SYS_shutdown] "sys_shutdown",
[SYS_getsockopt] "sys_getsockopt",
[SYS_setsockopt] "sys_setsockopt",
[SYS_sockclose] "sys_sockclose",
[SYS_connect] "sys_connect",
[SYS_getpeername] "sys_getpeername",
[SYS_getsockname] "sys_getsockname",
[SYS_select]  "sys_select",
[SYS_ioctl]   "sys_ioctl",
};

void
syscall(void)
{
  int num;

  num = proc->tf->eax;
  if(num > 0 && num < NELEM(syscalls) && syscalls[num]) {
    proc->tf->eax = syscalls[num]();

    #ifdef LWIP_SYSCALL_DEBUG
      if (num == 22 || num == 44 || (num >= 27 && num <= 41))
        cprintf("DEBUG: LWIP syscall for '%s' --> retval: %d\n", lwip_syscall_names[num], proc->tf->eax);
    #endif

  } else {
    cprintf("%d %s: unknown sys call %d\n",
            proc->pid, proc->name, num);
    proc->tf->eax = -1;
  }
}
