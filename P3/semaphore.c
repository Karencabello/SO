#include "semaphore.h"

void sem_init(Semaphore* s, int count) {
    pthread_mutex_init(&s->lock , NULL);
    pthread_cond_init(&s->wait, NULL);
    s->n = count;
}

void sem_wait(Semaphore* s) {
    pthread_mutex_lock(&s->lock);
    while(s->n <=0)    
        pthread_cond_wait(&s->wait, &s->lock);
    s->n--;
    pthread_mutex_unlock(&s->lock);

}

void sem_post(Semaphore* s) {
    pthread_mutex_lock(&s->lock);
    s->n++;
    pthread_cond_signal(&s->wait);
    pthread_mutex_unlock(&s->lock);
}
void sem_destroy(Semaphore* s){
    pthread_mutex_destroy(&s->lock);
    pthread_cond_destroy(&s->wait);
}
