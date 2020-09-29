#include "lwip/xv6/arch/sys_arch.h"
// #include "types.h"
#include "defs.h"
// #include "thread.h"
#include "param.h"
#include "mmu.h"
#include "proc.h"
#include "defs.h"


#ifndef MAXTHREADS
#define MAXTHREADS 16
#endif

extern struct proc* allocproc_wrapper(void);
extern struct proc* ptable_find_proc(char *tofind);
extern void procdump(void);

extern struct proc *initproc;
extern struct ptable_t ptable;

void thread_wrap(void (* thread)(void *arg), void *arg);
static int kptable_insert(kproc_t thread);
static kproc_t kptable_remove(int kpti);
static kproc_t kptable_find_thr(char *tofind);

static bool kptableinit = false; //0;

static struct {
    struct spinlock lock;
    kproc_t threads[MAXTHREADS];
    int size;
} kptable;


void
kptable_dump(void)
{
    acquire(&kptable.lock);

    static char *states[] = {
          [UNUSED]    "unused",
          [EMBRYO]    "embryo",
          [SLEEPING]  "sleep ",
          [RUNNABLE]  "runble",
          [RUNNING]   "run   ",
          [ZOMBIE]    "zombie",
          [MSLEEPING] "msleep"
    };
    int i;
    cprintf("\n____________________ KPROC TABLE (size: %d) ____________________\n\n", kptable.size);
    for (i = 0; i < MAXTHREADS; i++) {
        if (kptable.threads[i]) {
            kproc_t t = kptable.threads[i];
            cprintf("[%d] '%s':  proc=%p\n\tpid=%d  state=%s  sz=%u  killed=%d  parent='%s'  \n\tthr=%p  timeouts=%p  tsem=%p  chan=%p\n", 
                t->kptidx,
                t->p->name, 
                t->p,
                t->p->pid,
                states[t->p->state],
                t->p->sz,
                t->p->killed,
                (t->p->parent ? t->p->parent->name : "0"),
                t->p->thr,
                t->timeouts,
                t->tsem,
                t->p->chan
            );
        }
    }
    cprintf("_______________________________________________________________\n\n");

    release(&kptable.lock);
}

// #define USE_FORK  // for creating a child process for a new thread

kproc_t 
kproc_start(void (* runfunc)(void *arg), void *arg, int prio, void *data, char *name)
{
    // #ifdef KPROC_SEM_EXPERIMENTAL
    if (!kptableinit) {
        memset(&kptable, 0, sizeof(kptable));
        initlock(&kptable.lock, "kptable");
        kptableinit = true; //1;
        // cprintf("THREAD::kptable INITIALIZED\n");
    }
    // #endif

    kproc_t thr;
    struct proc *np;

    // int thr_exists = 0;
    // if (name) {
    //     thr = kptable_find_thr(name);
    //     if (thr) {
    //         while(thr->p->state == SLEEPING || thr->p->state == MSLEEPING ) {
    //             // cprintf("kproc_start waking up sleeping thread '%s'\n", name);
    //             wakeup(thr->p->chan);
    //             // wakeup(initproc);
    //             sys_sem_signal(thr->tsem);
    //         }
    //         // procdump();
    //         // kptable_dump();
    //         // assert(thr->p->state != SLEEPING);
    //         // assert(thr->p->state != MSLEEPING);
    //         // return thr;
    //         thr_exists = 1;
    //         np = thr->p;
    //     }
    // }

    // if (!thr_exists)
    // {
        thr = (kproc_t)kalloc();
        if (!thr)
            return NULL;
        thr->p = allocproc_wrapper();
        np = thr->p;
        if (!np)
            return NULL;

        // np->pgdir = setupkvm();
        np->pgdir = setupuserkvm();
    // }

        np->thr = thr;
        np->parent = initproc;
        // np->parent = ptable_find_proc("sh");
        np->sz = 0;
        np->chan = 0;
        np->killed = 0;
        
        // memset(np->context, 0, sizeof(np->context));
        memset(np->context, 0, sizeof(*np->context));

        thr->data = data;
        thr->timeouts.next = 0;
        thr->tsem = 0;

        if (name == 0)
            safestrcpy(np->name, "kernel thread", sizeof(np->name));
        else
            safestrcpy(np->name, name, sizeof(np->name));
    // }

    char *sp;
    sp = np->kstack + KSTACKSIZE - 1;

    sp -= sizeof np->tf;
    np->tf = (struct trapframe*) sp;
    sp -= 4;
    *(uint*)sp = (uint) exit;
    sp -= 4;
    *(uint*)sp = (uint) runfunc;
    sp -= 4;
    *(uint*)sp = (uint) arg;
    sp -= sizeof *np->context;
    np->context = (struct context*) sp;

    (np->context)->eip = (uint)thread_wrap;

    np->cwd = namei("/");
    np->state = RUNNABLE;

    if (!kptable_find_thr(np->name)){
        acquire(&kptable.lock);
        thr->kptidx = kptable_insert(thr);
        release(&kptable.lock);
    }
    return thr;
}

/* Insert new thread at the first available index and return that index */
static int 
kptable_insert(kproc_t thread)
{
    if (kptable.size == MAXTHREADS) {
        cprintf("[kptable_insert] ERROR: kproc table is full, could not insert new thread!\n");
        return -1;
    }
    int i;
    for (i = 0; i < MAXTHREADS; i++) {
        if (!kptable.threads[i]) {
            kptable.threads[i] = thread;
            kptable.size++;
            return i;
        }
    }
    return -1;
}

/* Remove thread from the specified index and return that thread to kill */
static kproc_t 
kptable_remove(int kpti)
{
    assert(kpti < MAXTHREADS);
    assert(kpti >= 0);
    if (kptable.size == 0) {
        cprintf("[kptable_remove] ERROR: kproc table is empty, could not remove thread!\n");
        return NULL;  // (kproc_t)0;
    }
    kproc_t deadthread = kptable.threads[kpti];
    if (!deadthread) {
        cprintf("[kptable_remove] ERROR: kproc table has no thread to remove at index %d!\n", kpti);
    } else {
        assert(deadthread->kptidx == kpti);
        kptable.threads[kpti] = NULL;
        kptable.size--;
        deadthread->kptidx = -1;
    }
    return deadthread;
}

void 
kproc_free(kproc_t thread)
{
    // cprintf("kproc_free called\n");
    struct proc *p = thread->p;

#ifdef KPROC_SEM_EXPERIMENTAL
    kproc_signal(p->name);
    sys_sem_free(thread->tsem);
    p->state = ZOMBIE;
#endif

    acquire(&kptable.lock);
    assert(kptable_remove(thread->kptidx) == thread);
    release(&kptable.lock);

    memset(thread, 0, sizeof(struct thread));
    kfree((char*)thread);

    p->thr = 0;
    p->state = UNUSED;
}

void 
thread_wrap(void (* runfunc)(void *arg), void *arg)
{
    // cprintf("thread beginning to serve its purpose:\n");
    release(&ptable.lock);
    runfunc(arg);
    exit();
}

static kproc_t
kptable_find_thr(char *tofind)
{
    int i, n = strlen(tofind);
    acquire(&kptable.lock);
    for (i = 0; i < MAXTHREADS; i++) {
        if (kptable.threads[i]) {
            int ni = strlen(kptable.threads[i]->p->name);
            if (ni == n && !strncmp(kptable.threads[i]->p->name, tofind, n)) {
                release(&kptable.lock);
                return kptable.threads[i];
            }
        }
    }
    release(&kptable.lock);
    // cprintf("[kptable_find_thr] ERROR: thread '%s' not found in kptable!\n");
    return NULL;
}

void
kproc_signal(char *thrname)
{
    kproc_t thread;// = kptable_find_thr(thrname);
    // assert(thread != NULL);

    while ((thread = kptable_find_thr(thrname))) {
        if (thread->tsem && thread->tsem->waiters) {
            // cprintf("[kproc_signal] posting sem wakeup signal for '%s'\n", thrname);
            // sem_post(thread->tsem);
            sys_sem_signal(thread->tsem);
        }
    }
}
