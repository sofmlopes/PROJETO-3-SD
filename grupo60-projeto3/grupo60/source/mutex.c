/**
 * Número do grupo: 60
 * João Vale 58159
 * Sofia Lopes 58175
 * Eduardo Santos 58185
*/

#include "mutex.h"

/**
 * Função de controlo de início de escrita.
*/
void begin_write(pthread_mutex_t write_mutex){   
    pthread_mutex_lock(&write_mutex); // Bloqueia o mutex de escrita
}

/**
 * Função de controlo de fecho de escrita.
*/
void end_write(pthread_mutex_t write_mutex){
    pthread_mutex_unlock(&write_mutex); // Desbloqueia o mutex de escrita após a operação
}

/**
 * Função de controlo de início de leitura
*/
void begin_read(pthread_mutex_t read_mutex, pthread_mutex_t write_mutex, int readers_count) {
    pthread_mutex_lock(&read_mutex); // Bloqueia o mutex de leitura
    readers_count++; // Incrementa o contador de leitores
    if (readers_count == 1) {
        pthread_mutex_lock(&write_mutex); // Se for o primeiro leitor, bloqueia o mutex de escrita
    }
    pthread_mutex_unlock(&read_mutex); // Desbloqueia o mutex de leitura
}

/**
 * Função de controlo de fecho de leitura
*/
void end_read(pthread_mutex_t read_mutex, pthread_mutex_t write_mutex, int readers_count) {
    pthread_mutex_lock(&read_mutex); // Bloqueia o mutex de leitura
    readers_count--; // Decrementa o contador de leitores
    if (readers_count == 0) {
        pthread_mutex_unlock(&write_mutex); // Se for o último leitor, desbloqueia o mutex de escrita
    }
    pthread_mutex_unlock(&read_mutex); // Desbloqueia o mutex de leitura após a operação
}