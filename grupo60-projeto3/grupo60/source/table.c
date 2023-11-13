/**
 * Número do grupo: 60
 * João Vale 58159
 * Sofia Lopes 58175
 * Eduardo Santos 58185
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "data.h"
#include "entry.h"
#include "list.h"
#include "table.h"
#include "table-private.h"

/* Função para criar e inicializar uma nova tabela hash, com n
 * linhas (n = módulo da função hash).
 * Retorna a tabela ou NULL em caso de erro.
 */
struct table_t *table_create(int n) {
    struct table_t *table = (struct table_t*)malloc(sizeof(struct table_t));
    
    if(table == NULL) {
        return NULL;
    }

    table->size = n;
    table->lists = (struct list_t**)malloc(n * sizeof(struct list_t*));
    if(table->lists == NULL) {
        free(table);
        return NULL;
    }

    for (int i = 0; i < table->size; i++) {
        table->lists[i] = list_create();
    }

    return table;    
}


/* Função que elimina uma tabela, libertando *toda* a memória utilizada
 * pela tabela.
 * Retorna 0 (OK) ou -1 em caso de erro.
 */
int table_destroy(struct table_t *table) {
    if(table == NULL) {
        return -1;
    }

    if(table->lists != NULL) {
        for (int i = 0; i < table->size; i++) {
            list_destroy(table->lists[i]);
        }

        free(table->lists);
    }

    free(table);

    return 0;
    
}


/* Função para adicionar um par chave-valor à tabela. Os dados de entrada
 * desta função deverão ser copiados, ou seja, a função vai criar uma nova
 * entry com *CÓPIAS* da key (string) e dos dados. Se a key já existir na
 * tabela, a função tem de substituir a entry existente na tabela pela
 * nova, fazendo a necessária gestão da memória.
 * Retorna 0 (ok) ou -1 em caso de erro.
 */
int table_put(struct table_t *table, char *key, struct data_t *value) {
    if(table == NULL || key == NULL || value == NULL) {
        return -1;
    }

    int index = hash_code(key, table->size);

    char *key_dup = (char *)malloc((strlen(key) + 1) * sizeof(char));
    if (key_dup == NULL) {
        return -1; 
    }
    strcpy(key_dup, key);

    struct data_t *data_duplicated = data_dup(value);

    if (key_dup == NULL || data_duplicated == NULL) {
        free(key_dup);
        data_destroy(data_duplicated);
        return -1;
    }

    // verificar se a chave já existe na lista e substituir se necessário.
    struct entry_t *existing_entry = list_get(table->lists[index], key_dup);
    if (existing_entry !=NULL) {
        entry_replace(existing_entry, key_dup, data_duplicated);
    } else {
        struct entry_t *new_entry = entry_create(key_dup, data_duplicated);
        if (new_entry == NULL) {
            free(key_dup);
            data_destroy(data_duplicated);
            return -1; 
        }

        int result = list_add(table->lists[index], new_entry);
        if (result == -1) {
            entry_destroy(new_entry);
            return -1; 
        }
    }

    return 0; 

}

/* Função que procura na tabela uma entry com a chave key. 
 * Retorna uma *CÓPIA* dos dados (estrutura data_t) nessa entry ou 
 * NULL se não encontrar a entry ou em caso de erro.
 */
struct data_t *table_get(struct table_t *table, char *key) {
    if(table == NULL || key == NULL) {
        return NULL;
    }

    int index = hash_code(key, table->size);

    struct entry_t *entry = list_get(table->lists[index], key);
    
    if(entry == NULL) {
        return NULL;
    }

    struct data_t *data = data_dup(entry->value);

    if(data == NULL) {
        entry_destroy(entry);
        return NULL;
    }

    return data;
}


/* Função que remove da lista a entry com a chave key, libertando a
 * memória ocupada pela entry.
 * Retorna 0 se encontrou e removeu a entry, 1 se não encontrou a entry,
 * ou -1 em caso de erro.
 */
int table_remove(struct table_t *table, char *key) {
    if(table == NULL || key == NULL) {
        return -1;
    }

    int index = hash_code(key, table->size);

    return list_remove(table->lists[index], key);
}


/* Função que conta o número de entries na tabela passada como argumento.
 * Retorna o tamanho da tabela ou -1 em caso de erro.
 */
int table_size(struct table_t *table) {
    if(table == NULL) {
        return -1;
    }

    int size = 0;

    for (int i = 0; i < table->size; i++) {
        size += list_size(table->lists[i]);
    }

    return size;
    
}

/* Função que constrói um array de char* com a cópia de todas as keys na 
 * tabela, colocando o último elemento do array com o valor NULL e
 * reservando toda a memória necessária.
 * Retorna o array de strings ou NULL em caso de erro.
 */
char **table_get_keys(struct table_t *table) {
    if (table == NULL) {
        return NULL;
    }

    int num_entries = table_size(table);

    char **keys = (char **)malloc((num_entries + 1) * sizeof(char *));
    if (keys == NULL) {
        return NULL; 
    }

    int index = 0;

    /*percorrer cada lista da tabela, copiando as keys para o array de char* criado*/
    for (int i = 0; i < table->size; i++) {
        char **list_keys = list_get_keys(table->lists[i]);
        
        //tratamento de erros
        if (list_keys == NULL) {
            for (int j = 0; j < index; j++) {
                free(keys[j]);
            }
            free(keys);
            return NULL;
        }
    

        for (int j = 0; list_keys[j] != NULL; j++) {
            keys[index] = strdup(list_keys[j]);
            
            //tratamento de erros
            if (keys[index] == NULL) {
                for (int k = 0; k < index; k++) {
                    free(keys[k]);
                }
                free(keys);
                list_free_keys(list_keys); 
                return NULL;
            }

            index++;
        }

        list_free_keys(list_keys);
    }

    keys[num_entries] = NULL;
    return keys;
}


/* Função que liberta a memória ocupada pelo array de keys obtido pela 
 * função table_get_keys.
 * Retorna 0 (OK) ou -1 em caso de erro.
 */
int table_free_keys(char **keys) {
    if(keys == NULL) {
        return -1;
    }
    for (int i = 0; keys[i] != NULL; i++) {
        free(keys[i]);
        
    }
    free(keys);

    return 0;
}