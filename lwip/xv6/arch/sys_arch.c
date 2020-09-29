#include "sys_arch.h"
#include "../../types.h"
#include "../../defs.h"
#include "../../sem.h"
#include "../../include/lwip/sys.h"
#include "../../include/lwip/err.h"
#include "../../spinlock.h"
#include "../../assert.h"
#include "../../param.h"
#include "../../mmu.h"
#include "../../proc.h"
#include "../../thread.h"

#include "../../include/lwip/memp.h"

#ifndef NULL
#define NULL 0
#endif

#ifndef MEM_SYS_TIMEOUT
#define MEM_SYS_TIMEOUT 10
#endif
#ifndef MEM_SYS_SEM
#define MEM_SYS_SEM 11
#endif
#ifndef MEM_SYS_MBOX
#define MEM_SYS_MBOX 12
#endif

/*static*/ int sa_init = 0;

// Defined in sys.h
struct sys_timeout default_timeo;

// Default linked list of timeouts for lwIP
// struct sys_timeouts lwip_system_timeouts;   // = NULL;  
struct sys_timeouts *lwip_system_timeouts;

static void
setup_default_timeouts()  
{
    lwip_system_timeouts = (struct sys_timeouts *)kalloc();
    default_timeo.next = 0;
    default_timeo.time = 3000; // ~3 sec.
    default_timeo.h = 0;
    default_timeo.arg = 0;

    lwip_system_timeouts->next = &default_timeo;

    sa_init = 1;
    cprintf("SYS_ARCH::lwip_system_timeouts INITIALIZED\n");
}

// The "count" argument specifies the initial state of the semaphore (which is either 0 or 1)
sys_sem_t 
sys_sem_new(u8_t count)
{
    sys_sem_t sem = (sys_sem_t)kalloc();
    if (!sem)
        return SYS_SEM_NULL;
    sem_init(sem, count);
    return sem;
}

void 
sys_sem_free(sys_sem_t sem)
{
    if (sem) {
        sem_destroy(sem);
        kfree((char*)sem);
    }
}

void 
sys_sem_signal(sys_sem_t sem)
{
    if (sem)
        sem_post(sem);
}

u32_t 
sys_arch_sem_wait(sys_sem_t sem, u32_t timeout)
{
    if (!sem)
        return -1;

    int s = millitime(), p;
    int ret;

    if (timeout == 0)
    {
        sem_wait(sem);
        return 0;
    }

    ret = sem_timedwait(sem, timeout);
    
    p = millitime() - s;
    if (ret == 0)
        return p;
    else
        return SYS_ARCH_TIMEOUT;
}

sys_mbox_t 
sys_mbox_new(void)
{   
    sys_mbox_t mbox = (sys_mbox_t)kalloc();

    if (!mbox)
        return SYS_MBOX_NULL;
    initlock(&mbox->lock, "mbox");

    mbox->free = (sem_t *)kalloc(); 
    mbox->queued = (sem_t *)kalloc();

    sem_init(mbox->free, NSLOTS);
    sem_init(mbox->queued, 0);
    mbox->count = 0;
    mbox->head = -1;
    mbox->next = 0;
    return mbox;
};

void 
sys_mbox_free(sys_mbox_t mbox)
{
    if (!mbox)
        return;
    acquire(&mbox->lock);
    sem_destroy(mbox->free);
    sem_destroy(mbox->queued);
    if (mbox->count != 0)
        cprintf("sys_mbox_free: Warning: mbox not free\n");
    release(&mbox->lock);

    kfree((char*)mbox);
}

void 
sys_mbox_post(sys_mbox_t mbox, void *msg)
{
    if (!mbox)
        return;
    sem_wait(mbox->free);
    acquire(&mbox->lock);
    if (mbox->count == NSLOTS)
    {
        release(&mbox->lock);
        return;
    }
    int slot = mbox->next;
    mbox->next = (slot + 1) % NSLOTS;
    mbox->slots[slot] = msg;
    mbox->count++;
    if (mbox->head == -1)
        mbox->head = slot;

    sem_post(mbox->queued);
    release(&mbox->lock);
}

u32_t 
sys_arch_mbox_fetch(sys_mbox_t mbox, void **msg, u32_t timeout)
{
    // cprintf("[sys_arch_mbox_fetch]\n");
    if (!mbox)
        return SYS_ARCH_TIMEOUT;
    u32_t waited = sys_arch_sem_wait(mbox->queued, timeout);
    acquire(&mbox->lock);
    if (waited == SYS_ARCH_TIMEOUT)
    {
        release(&mbox->lock);
        return waited;
    }

    int slot = mbox->head;
    if (slot == -1)
    {
        release(&mbox->lock);
        cprintf("fetch failed!\n");
        return SYS_ARCH_TIMEOUT;
    }

    if (msg)
        *msg = mbox->slots[slot];

    mbox->head = (slot + 1) % NSLOTS;
    mbox->count--;
    if (mbox->count == 0)
        mbox->head = -1;

    sem_post(mbox->free);
    release(&mbox->lock);
    return waited;
}

u32_t 
sys_jiffies(void)
{
    return millitime();
}

sys_thread_t 
sys_thread_new(void (* thread)(void *arg), void *arg, int prio, char *name)
{
    return kproc_start(thread, arg, prio, 0, name);
}

struct sys_timeouts *
sys_arch_timeouts(void)
{
    if (!sa_init) setup_default_timeouts();

    if (proc && proc->thr) {
        // cprintf("[sys_arch_timeouts] proc:%s\n", proc->name);
        return &proc->thr->timeouts;
    }
    else
        return lwip_system_timeouts;
}
// If we choose to use threads, then we will need to give each
// thread its own sys_timeouts structure, and return that
// depending on what thread calls this function.
// (this has already been incorpated into the thread struct in thread.h,
// but will require the E1000 to run 'kproc_start')
    
