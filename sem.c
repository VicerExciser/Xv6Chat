////  Implemented for xv6Chat  ////
#include "assert.h"
// #include "types.h"
#include "string.h"
#include "defs.h"
#include "sem.h"
#include "mmu.h"
#include "proc.h"

// struct sem {
//     struct spinlock lock;
//     int val;        // count
//     int waiters;
//     // int valid;
// };

static int semn = 0;
static char *semnamebase = "sem";

int 
sem_init(sem_t *sem, unsigned int value)
{
    // cprintf("[sem_init]\n");
    assert(value >= 0);

    char num[3];
    itoa(semn++, num, 10);
    // strcpy(name, semnamebase);
    safestrcpy(sem->name, semnamebase, sizeof(sem->name));
    strcat(sem->name, num);
    // cprintf("[sem_init] %s\n", sem->name);

    initlock(&sem->lock, sem->name);
    sem->val = value;
    sem->waiters = 0;
    // sem->valid = 1;
    return 0;
}

int 
sem_destroy(sem_t *sem)
{
    //cprintf("[sem_destroy] '%s'\n", sem->name);
    assert(sem->waiters == 0);
    // kfree((char*)sem);
    semn--;
    return 0;
}

void 
sem_post(sem_t *sem)
{
    // cprintf("[sem_post]\n");
    acquire(&sem->lock);
    sem->val++;
    if ((sem->waiters) && (sem->val > 0))
    {
        wakeup_one(sem); // XXX maybe wakeup?
        // wakeup(sem);
    }
    release(&sem->lock);
}

void 
sem_wait(sem_t *sem)
{
    // cprintf("[sem_wait]\n");
    acquire(&sem->lock);
    while (sem->val == 0)
    {
        sem->waiters++;
        sleep(sem, &sem->lock);
        sem->waiters--;
    }
    sem->val--;
    release(&sem->lock);
    // cprintf("[sem_wait] RETURNING\n");
}

// #define PING_MSLEEP_DEBUG 

int 
sem_timedwait(sem_t *sem, int timo)
{
#ifdef KPROC_SEM_EXPERIMENTAL
    if (proc && proc->name) {
        if (!proc->thr->tsem || proc->thr->tsem != sem) {
            cprintf("[sem_timedwait] attaching sem %p to thread '%s'\n", sem, proc->name);
            proc->thr->tsem = sem;
        }
    }
#endif

    #ifdef PING_MSLEEP_DEBUG
        int ping_spin_cnt = 0;
    #endif

    int ret;

    acquire(&sem->lock);
    for (ret = 0; sem->val == 0 && ret == 0;)
    {
        sem->waiters++;
        ret = msleep_spin(sem, &sem->lock, timo);
        #ifdef PING_MSLEEP_DEBUG
            if (proc && proc->name && !strncmp(proc->name, "ping thread", strlen(proc->name)))
                ping_spin_cnt++;
        #endif
        sem->waiters--;

#ifdef KPROC_SEM_EXPERIMENTAL
        if (proc && proc->killed){
            sem->val = 0;
            ret = 1;
            break;
        }
#endif

    }

    if (sem->val > 0)
    {
        sem->val--;
        ret = 0;
    }

    release(&sem->lock);
    #ifdef PING_MSLEEP_DEBUG
        if (proc && proc->name && !strncmp(proc->name, "ping thread", strlen(proc->name)))
            cprintf("[sem_timedwait] PING THREAD RETURNING AFTER %d MSLEEP_SPINS\n", ping_spin_cnt);
    #endif
    return ret;
}

int 
sem_trywait(sem_t *sem)
{
    // cprintf("[sem_trywait]\n");
    int ret;

    acquire(&sem->lock);
    if (sem->val > 0)
    {
        sem->val--;
        ret = 1;
    } else {
        ret = 0;
    }
    release(&sem->lock);
    return ret;
}

int 
sem_value(sem_t *sem)
{
    // cprintf("[sem_value]\n");
    int ret;

    acquire(&sem->lock);
    ret = sem->val;
    release(&sem->lock);
    return ret;
}

int 
sem_size()
{
    // cprintf("[sem_size]\n");
    return sizeof(struct sem);
}
