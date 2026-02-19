#pragma once
#include <pthread.h>

/*
Semaphore is like a generalised lock, which allows n threads to be in the same region. When n = 1, it is equivalent to a lock
It is mostly used for resource management (like buffers with N entries), and also to ensure order in the code 
(that some parts of the code are executed before others).

Sem_wait = mutex_lock (decreases n if > 0, otherwise waits)
sem_post => mutex_unlock (if n = 0, and there is a thread waiting, lets him continue, otherwise, increments n).

There exists
*/

typedef struct {
    pthread_mutex_t lock;
    pthread_cond_t wait;
    int n;
} Semaphore;

void sem_init(Semaphore* s, int count);
void sem_wait(Semaphore* s);
void sem_post(Semaphore* s);
void sem_destroy(Semaphore* s);
