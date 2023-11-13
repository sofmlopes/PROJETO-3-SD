/**
 * Número do grupo: 60
 * João Vale 58159
 * Sofia Lopes 58175
 * Eduardo Santos 58185
*/

#include "sdmessage.pb-c.h"
#include "stdio.h"
#include "string.h"
#include "stdlib.h"
#include "data.h"

/* Função que cria um novo elemento de dados data_t e que inicializa 
 * os dados de acordo com os argumentos recebidos, sem necessidade de
 * reservar memória para os dados.	
 * Retorna a nova estrutura ou NULL em caso de erro.
 */
struct data_t *data_create(int size, void *data){
	struct data_t *new_data;
	
	if(data == NULL || size < 1)
	   return NULL;


	new_data = (struct data_t*) malloc(sizeof(struct data_t));
	if (new_data == NULL)
        return NULL;

    new_data->datasize = size;
    new_data->data = data;

    return new_data;
};


/* Função que elimina um bloco de dados, apontado pelo parâmetro data,
 * libertando toda a memória por ele ocupada.
 * Retorna 0 (OK) ou -1 em caso de erro.
 */
int data_destroy(struct data_t *data){
    if(data == NULL)
        return -1;
    free(data->data);
    free(data);
    return 0;
};

/* Função que duplica uma estrutura data_t, reservando a memória
 * necessária para a nova estrutura.
 * Retorna a nova estrutura ou NULL em caso de erro.
 */
struct data_t *data_dup(struct data_t *data){

    if(data ==  NULL || data->data == NULL || data->datasize < 1)
	   return NULL;
    
    struct data_t *dup_data = (struct data_t *)malloc(sizeof(struct data_t));

    if(dup_data == NULL)
        return NULL;
    
    dup_data->datasize = data->datasize;

    dup_data->data = malloc(data->datasize);
    if (dup_data->data == NULL) {
        data_destroy(data);
        return NULL;
    }
    memcpy(dup_data->data, data->data, data->datasize);

    return dup_data;
};

/* Função que substitui o conteúdo de um elemento de dados data_t.
 * Deve assegurar que liberta o espaço ocupado pelo conteúdo antigo.
 * Retorna 0 (OK) ou -1 em caso de erro.
 */
 int data_replace(struct data_t *data, int new_size, void *new_data) {
    if (data == NULL)
        return -1;

    free(data->data);

    data->datasize = new_size;
    data->data = new_data;

    return 0;
}


