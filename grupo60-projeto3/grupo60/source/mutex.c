#include "mutex.h"

/**
 * Função de controlo de início de escrita.
*/
int begin_write(int thread_id){
    pthread_mutex_lock(&m);
    while(writers == 0){
        pthread_cond_wait(&c, &m);
    }
    writers++;
    pthread_mutex_unlock(&m);
}

/**
 * Função de controlo de fecho de escrita.
*/
void end_write(){
    pthread_mutex_lock(&m);
    writers--;
    pthread_cond_broadcast(&c);
    pthread_mutex_unlock(&m);
}