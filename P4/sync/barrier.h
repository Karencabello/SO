#pragma once
#include <pthread.h>

typedef struct {
    pthread_mutex_t lock;
    pthread_cond_t wait;
    int n;
} Barrier;
void barrier_reset(Barrier* s, int count);

void barrier_init(Barrier* s, int count);
void barrier_wait(Barrier* s);
void barrier_destroy(Barrier* s);
