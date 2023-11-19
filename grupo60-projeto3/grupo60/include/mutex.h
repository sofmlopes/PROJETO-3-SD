#ifndef _MUTEX_H
#define _MUTEX_H

#include <pthread.h>

/**
 * Função de controlo de início de escrita.
*/
void begin_write(pthread_mutex_t write_mutex);

/**
 * Função de controlo de fecho de escrita.
*/
void end_write(pthread_mutex_t write_mutex);

/**
 * Função de controlo de início de leitura
*/
void begin_read(pthread_mutex_t read_mutex, pthread_mutex_t write_mutex, int readers_count);

/**
 * Função de controlo de fecho de leitura
*/
void end_read(pthread_mutex_t read_mutex, pthread_mutex_t write_mutex, int readers_count);


#endif