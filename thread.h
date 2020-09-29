#ifndef __XV6_THREAD_H__
#define __XV6_THREAD_H__

#ifndef SYS_TIMEOUTS_DEFINED
#define SYS_TIMEOUTS_DEFINED

struct sys_timeouts {
    struct sys_timeout *next;
};

#endif // SYS_TIMEOUTS_DEFINED

#include "sem.h"

struct thread {
    struct proc *p;
    void *data;
    struct sys_timeouts timeouts;
    sem_t *tsem;
    int kptidx;		// thread's index in the kptable (-1 if dead)
    // void (* signal)(void);
};

typedef struct thread * kproc_t;

kproc_t kproc_start(void (* runfunc)(void *arg), 
        void *arg, int prio, void *data, char *name);

void kptable_dump(void);

void kproc_free(kproc_t thread);

void kproc_signal(char *thrname);


#endif /* __XV6_THREAD_H__ */