////  Implemented for xv6Chat  ////

#ifndef __XV6_SEM_H__
#define __XV6_SEM_H__

// #define KPROC_SEM_EXPERIMENTAL

#include "spinlock.h"
struct sem {
    struct spinlock lock;
    int val;        // count (either 0 or 1)
    int waiters;
    char name[7];
};
// struct sem;
typedef struct sem sem_t;

int sem_init(sem_t *sem, unsigned int value);
int sem_destroy(sem_t *sem);
void sem_post(sem_t *sem);
void sem_wait(sem_t *sem);
int sem_timedwait(sem_t *sem, int timo);
int sem_trywait(sem_t *sem);
int sem_value(sem_t *sem);
int sem_size();

#endif /* __XV6_SEM_H__ */