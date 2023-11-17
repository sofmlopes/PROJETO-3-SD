#ifndef _STATS_H
#define _STATS_H /* Módulo estatísticas */

/* Estrutura que define os dados.
 */
struct statistics_t {
	int num_operations; /* Número total de operações na tabela executadas no servidor */
    unsigned long time; /* Tempo total acumulado gasto na execução de operações na tabela */
    int num_clients; /* Número de clientes atualmente ligados ao servidor */
};

/* Função que cria e inicializa uma nova estrutura statistics_t.
 * Retorna a lista ou NULL em caso de erro.
 */
struct statistics_t *stats_create(); 

/* Função que elimina uma estrutura statistics_t.
 * Retorna 0 (OK) ou -1 em caso de erro.
 */
int stats_destroy(struct statistics_t *stats);

#endif