/**
 * Número do grupo: 60
 * João Vale 58159
 * Sofia Lopes 58175
 * Eduardo Santos 58185
*/

#include <stdio.h>
#include <stdlib.h>

#include "stats.h"

/* Função que cria e inicializa uma nova estrutura statistics_t.
 * Retorna a lista ou NULL em caso de erro.
 */
struct statistics_t *stats_create(){

    struct statistics_t *new_stats;

    new_stats = (struct statistics_t *) malloc(sizeof(struct statistics_t));

    if (new_stats == NULL)
        return NULL;
    
    new_stats->num_clients=0;
    new_stats->num_operations=0;
    new_stats->time=0.0;

    return new_stats;
}

/* Função que elimina uma estrutura statistics_t.
 * Retorna 0 (OK) ou -1 em caso de erro.
 */
int stats_destroy(struct statistics_t *stats){

    if (stats == NULL)
        return -1;

    free(stats);
    
    return 0;
}