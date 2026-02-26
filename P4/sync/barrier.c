#include "barrier.h"

void barrier_init(Barrier* s, int count) {
    pthread_mutex_init(&s->lock, NULL);
    pthread_cond_init(&s->wait, NULL);
    s->n = count;
}

void barrier_reset(Barrier* s, int count) {
    s->n = count;
}

void barrier_wait(Barrier* s){
    pthread_mutex_lock(&s->lock);
    --s->n;
    while(s->n  >0)    
        pthread_cond_wait(&s->wait, &s->lock);
    pthread_cond_signal(&s->wait); // Or broadcast
    pthread_mutex_unlock(&s->lock);

}
void barrier_destroy(Barrier* s) {
    pthread_mutex_destroy(&s->lock);
    pthread_cond_destroy(&s->wait);

}
