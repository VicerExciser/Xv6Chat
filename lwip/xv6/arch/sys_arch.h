#ifndef XV6_LWIP_SYS_ARCH_H_
#define XV6_LWIP_SYS_ARCH_H_

#define SYS_ARCH_DECL_PROTECT(lev)
#define SYS_ARCH_PROTECT(lev)
#define SYS_ARCH_UNPROTECT(lev)

#include "../../../types.h"
#include "../../../spinlock.h"
#include "../../../thread.h"
// #include "../../../sem.h"
#include "cc.h"


#define SYS_MBOX_NULL 0
#define SYS_SEM_NULL 0

typedef sem_t* sys_sem_t;

// struct mbox;
#define NSLOTS 128

struct mbox {
    struct spinlock lock;
    sys_sem_t free, queued;
    int count, head, next;
    void *slots[NSLOTS];
    // int valid;
    char name[5];   // <-- using for ping debug
};
typedef struct mbox* sys_mbox_t;

typedef kproc_t sys_thread_t;

sys_sem_t sys_sem_new(u8_t count);

void sys_sem_free(sys_sem_t sem);
void sys_sem_signal(sys_sem_t sem);
u32_t sys_arch_sem_wait(sys_sem_t sem, u32_t timeout);

sys_mbox_t sys_mbox_new(void);
void sys_mbox_free(sys_mbox_t mbox);
void sys_mbox_post(sys_mbox_t mbox, void *msg);
u32_t sys_arch_mbox_fetch(sys_mbox_t mbox, void **msg, u32_t timeout);

struct sys_timeouts *sys_arch_timeouts(void);

sys_thread_t sys_thread_new(void (* thread)(void *arg), void *arg, int prio, char *name);


// xv6Chat/lwip/xv6/arch/sys_arch.h

#endif // XV6_LWIP_SYS_ARCH_H_
