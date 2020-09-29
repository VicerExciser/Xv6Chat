#ifndef __XV6_USER__
#define __XV6_USER__

// #include "types.h"
struct stat;
struct rtcdate;

#include "lwip/nettypes.h"

#ifndef uint
#define uint unsigned int
#endif

// system calls
int fork(void);
int exit(void) __attribute__((noreturn));
int wait(void);
int pipe(int*);
int write(int, void*, int);
int read(int, void*, int);
int close(int);
int kill(int);
int exec(char*, char**);
int open(char*, int);
int mknod(char*, short, short);
int unlink(char*);
int fstat(int fd, struct stat*);
int link(char*, char*);
int mkdir(char*);
int chdir(char*);
int dup(int);
int getpid(void);
char* sbrk(int);
int sleep(int);
int uptime(void);

int arp(char*, char*, char*, int);
int arpserv(char*);
int arp_receive(char*, int);

// Select system call waits if all fds block
// @param {fd_set *} readfds - Set of fds to read
// @param {fd_set *} writefds - Set of fds to write
// @return {int} -1 on error
// int select(int, fd_set *, fd_set *);

int umsec(void);
int pkill(char*);
int send_ping(char*);
int getc(void);
void* find_proc(char* procname);


// lwIP Networking System Calls for sockets.c
int accept(int s, struct sockaddr *addr, socklen_t *addrlen);
int bind(int s, struct sockaddr *name, socklen_t namelen);
int shutdown(int s, int how);
int getpeername(int s, struct sockaddr *name, socklen_t *namelen);
int getsockname(int s, struct sockaddr *name, socklen_t *namelen);
int getsockopt(int s, int level, int optname, void *optval, socklen_t *optlen);
int setsockopt(int s, int level, int optname, const void *optval, socklen_t optlen);
int sockclose(int s);
int connect(int s, struct sockaddr *name, socklen_t namelen);
int listen(int s, int backlog);
int recv(int s, void *mem, int len, unsigned int flags);
int recvfrom(int s, void *mem, int len, unsigned int flags,
      struct sockaddr *from, socklen_t *fromlen);
int send(int s, void *dataptr, int size, unsigned int flags);
int sendto(int s, void *dataptr, int size, unsigned int flags,
    struct sockaddr *to, socklen_t tolen);
int socket(int domain, int type, int protocol);
int select(int maxfdp1, fd_set *readset, fd_set *writeset, fd_set *exceptset,
            struct timeval *timeout);
int ioctl(int s, long cmd, void *argp);

// ulib.c
int stat(char*, struct stat*);
char* strcpy(char*, char*);
void *memmove(void*, void*, int);
char* strchr(const char*, char c);
int strcmp(const char*, const char*);
void printf(int, char*, ...);
char* gets(char*, int max);
uint strlen(char*);
void* memset(void*, int, uint);
void* malloc(uint);
void free(void*);
int atoi(const char*);

char* strcat(char*, const char*);
char* itoa(int value, char* buffer, int base);


#endif /* __XV6_USER__ */
