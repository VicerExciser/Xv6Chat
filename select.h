#ifndef SELECT_H
#define SELECT_H

#include "types.h"
#include "param.h"

struct spinlock;

struct selproc
{
  int * sel[NSELPROC];
  struct spinlock * lk[NSELPROC];
  int selcount;
};

void initselproc(struct selproc *);
void clearselid(struct selproc *, int *);
void addselid(struct selproc *, int*, struct spinlock * lk);
void wakeupselect(struct selproc *);

static inline void _fd_set(int fd, uint* set)
{
    *set |= (1 << fd);
}

static inline int _fd_isset(int fd, uint* set)
{
    return *set & (1 << fd);
}

static inline void _fd_clr(int fd, uint* set)
{
    *set &= ~(1 << fd);
}

static inline void _fd_zero(uint* set)
{
    *set = 0;
}

#ifndef FD_SET
#define FD_SET(fd, set) _fd_set(fd, set)
#endif
#ifndef FD_ISSET
#define FD_ISSET(fd, set) _fd_isset(fd,set)
#endif
#ifndef FD_CLR
#define FD_CLR(fd, set) _fd_clr(fd, set)
#endif
#ifndef FD_ZERO
#define FD_ZERO(set) _fd_zero(set)
#endif

#endif
