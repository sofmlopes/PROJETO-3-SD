#ifndef _MUTEX_H
#define _MUTEX_H

#include <pthread.h>

pthread_mutex_t m = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t c = PTHREAD_COND_INITIALIZER;
int writers = 1;


/**
 * Função de controlo de início de escrita.
*/
int begin_write(int thread_id);

/**
 * Função de controlo de fecho de escrita.
*/
void end_write();


#endif