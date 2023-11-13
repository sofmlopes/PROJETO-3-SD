/**
 * Número do grupo: 60
 * João Vale 58159
 * Sofia Lopes 58175
 * Eduardo Santos 58185
*/

#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "data.h"
#include "entry.h"
#include "list.h"
#include "list-private.h"

/* Função que cria e inicializa uma nova lista (estrutura list_t a
 * ser definida pelo grupo no ficheiro list-private.h).
 * Retorna a lista ou NULL em caso de erro.
 */
struct list_t *list_create(){

    struct list_t *new_list;

    new_list = (struct list_t *) malloc(sizeof(struct list_t));

    if (new_list == NULL)
        return NULL;
    
    new_list->head = NULL;
    new_list->size = 0;

    return new_list;
}

/* Função que elimina uma lista, libertando *toda* a memória utilizada
 * pela lista.
 * Retorna 0 (OK) ou -1 em caso de erro.
 */
int list_destroy(struct list_t *list){

    if (list == NULL)
        return -1;

    struct node_t *first = list->head;

    while (first != NULL){
        struct node_t *tmp= first;
        first=first->next;
        entry_destroy(tmp->entry);
    }

    free(list);
    
    return 0;
}

/* Função que adiciona à lista a entry passada como argumento.
 * A entry é inserida de forma ordenada, tendo por base a comparação
 * de entries feita pela função entry_compare do módulo entry e
 * considerando que a entry menor deve ficar na cabeça da lista.
 * Se já existir uma entry igual (com a mesma chave), a entry
 * já existente na lista será substituída pela nova entry,
 * sendo libertada a memória ocupada pela entry antiga.
 * Retorna 0 se a entry ainda não existia, 1 se já existia e foi
 * substituída, ou -1 em caso de erro.
 */
int list_add(struct list_t *list, struct entry_t *entry) {
    if (list == NULL || entry == NULL) {
        return -1; 
    }

    struct node_t *new_node = (struct node_t *)malloc(sizeof(struct node_t));
    if (new_node == NULL) {
        return -1; 
    }

    new_node->entry = entry;
    new_node->next = NULL;

    if (list->head == NULL) {
        list->head = new_node;
        list->size = 1;
        return 0; 
    }

    struct node_t *current_node = list->head;
    struct node_t *previous_node = NULL;

    while (current_node != NULL && entry_compare(current_node->entry, entry) == -1) {
        previous_node = current_node;
        current_node = current_node->next;
    }

    if (current_node != NULL && entry_compare(current_node->entry, entry) == 0) {
        current_node->entry = entry;
        free(new_node);
        return 1; 
    }

    if (previous_node == NULL) {
        new_node->next = list->head;
        list->head = new_node;
    } else {
        new_node->next = current_node;
        previous_node->next = new_node;
    }

    list->size++;
    return 0; 
}

/* Função que elimina da lista a entry com a chave key, libertando a
 * memória ocupada pela entry.
 * Retorna 0 se encontrou e removeu a entry, 1 se não encontrou a entry,
 * ou -1 em caso de erro.
 */
int list_remove(struct list_t *list, char *key){

    if(list == NULL || key == NULL) 
        return -1;

    struct node_t *current_node = list->head;
    struct node_t *previous_node = NULL;

    // percorrer ate encontrar chave ou chegar ao final
    while(current_node !=NULL && strcmp(current_node->entry->key, key) != 0){
        previous_node=current_node;
        current_node=current_node->next;
    }

    if(current_node==NULL)
        return 1;  

    if (previous_node == NULL) {
        list->head = current_node->next;
    } else {
        previous_node->next = current_node->next;
    }

    entry_destroy(current_node->entry);
    free(current_node);
    list->size--;
    return 0;
}

/* Função que obtém da lista a entry com a chave key.
 * Retorna a referência da entry na lista ou NULL se não encontrar a
 * entry ou em caso de erro.
*/
struct entry_t *list_get(struct list_t *list, char *key){

    if(list==NULL || key==NULL)
        return NULL;

    struct node_t *current_node = list->head;

    while(current_node !=NULL){
        if(strcmp(current_node->entry->key, key) == 0){
            return current_node->entry;
        }
        current_node=current_node->next;
    }
    return NULL;
}

/* Função que conta o número de entries na lista passada como argumento.
 * Retorna o tamanho da lista ou -1 em caso de erro.
 */
int list_size(struct list_t *list){
    if(list==NULL)
        return -1;
    return list->size;
}

/* Função que constrói um array de char* com a cópia de todas as keys na 
 * lista, colocando o último elemento do array com o valor NULL e
 * reservando toda a memória necessária.
 * Retorna o array de strings ou NULL em caso de erro.
 */
char **list_get_keys(struct list_t *list){

    if (list == NULL || list->size == 0) 
        return NULL; 

    char** keys= (char **) malloc((list->size+1)*sizeof(char *));
    int i = 0;

    if(keys==NULL)
        return NULL;

    struct node_t *current_node = list->head;

    while (current_node != NULL) {

        keys[i] = strdup(current_node->entry->key);
        if (keys[i] == NULL) {
            // erro de alocação de memória para a cópia da chave
            for (int j = 0; j < i; j++) {
                free(keys[j]); 
            }
            free(keys); 
            return NULL;
        }
        current_node = current_node->next;
        i++;
    }

    keys[i] = NULL; 
    return keys;

}

/* Função que liberta a memória ocupada pelo array de keys obtido pela 
 * função list_get_keys.
 * Retorna 0 (OK) ou -1 em caso de erro.
 */
int list_free_keys(char **keys){

    if(keys == NULL)
        return -1;

    int i = 0;
    while(keys[i] != NULL){
        free(keys[i]);
        i++;
    }

    free(keys);
    return 0;

}

