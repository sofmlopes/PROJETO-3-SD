#ifndef _MUTEX_H
#define _MUTEX_H

#include <pthread.h>

extern pthread_mutex_t m;
extern pthread_cond_t c;
extern int writers;

/**
 * Função de controlo de início de escrita.
*/
int begin_write(int thread_id);

/**
 * Função de controlo de fecho de escrita.
*/
void end_write();


#endif