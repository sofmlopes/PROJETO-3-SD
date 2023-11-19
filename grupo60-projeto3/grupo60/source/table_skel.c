/**
 * Número do grupo: 60
 * João Vale 58159
 * Sofia Lopes 58175
 * Eduardo Santos 58185
*/

#include <string.h>
#include <stdlib.h>
#include <sys/time.h>

#include "data.h"
#include "mutex.h"
#include "network_server.h"
#include "sdmessage.pb-c.h"
#include "stats.h"
#include "table.h"
#include "table_skel.h"

/*
* Funcão auxiliar, que devolve o tempo atual em microsegundos.
*/
unsigned long get_time_micros(){

    struct timeval current_time;
    gettimeofday(&current_time, NULL);
    return (unsigned long) (1000000 * current_time.tv_sec + current_time.tv_usec);
}

/* Inicia o skeleton da tabela.
 * O main() do servidor deve chamar esta função antes de poder usar a
 * função invoke(). O parâmetro n_lists define o número de listas a
 * serem usadas pela tabela mantida no servidor.
 * Retorna a tabela criada ou NULL em caso de erro.
 */
struct table_t *table_skel_init(int n_lists){

    struct table_t *new_table = table_create(n_lists);

    return new_table;

}

/* Liberta toda a memória ocupada pela tabela e todos os recursos 
 * e outros recursos usados pelo skeleton.
 * Retorna 0 (OK) ou -1 em caso de erro.
 */
int table_skel_destroy(struct table_t *table){

    return table_destroy(table);
}

/* Executa na tabela table a operação indicada pelo opcode contido em msg 
 * e utiliza a mesma estrutura MessageT para devolver o resultado.
 * Retorna 0 (OK) ou -1 em caso de erro.
*/
int invoke(MessageT *msg, struct table_t *table){

    if (msg == NULL || table == NULL) 
        return -1;
    
    if(msg->opcode == MESSAGE_T__OPCODE__OP_PUT && msg->c_type == MESSAGE_T__C_TYPE__CT_ENTRY){

        unsigned long init_time = get_time_micros();

        if(msg->entry->value.len>0 && msg->entry->value.data != NULL){

            struct data_t *data = data_create(msg->entry->value.len, msg->entry->value.data); 
            
            if (data != NULL){
                
                if(table_put(table,msg->entry->key,data) == 0){
                    msg->opcode = MESSAGE_T__OPCODE__OP_PUT+1;
                    msg->c_type = MESSAGE_T__C_TYPE__CT_NONE;
                    stats->num_operations++;
                    stats->time += (unsigned long) get_time_micros()-init_time;
                }
            }
        }

        else{
            msg->opcode = MESSAGE_T__OPCODE__OP_ERROR;
            msg->c_type = MESSAGE_T__C_TYPE__CT_NONE; 
        }
    }

    else if(msg->opcode == MESSAGE_T__OPCODE__OP_GET && msg->c_type == MESSAGE_T__C_TYPE__CT_KEY){

        unsigned long init_time = get_time_micros();

        if (msg->key != NULL){

            struct data_t *data = table_get(table,msg->key);

            if (data != NULL) {

                msg->opcode = MESSAGE_T__OPCODE__OP_GET + 1;
                msg->c_type = MESSAGE_T__C_TYPE__CT_VALUE;
                msg->value.len = data->datasize;
                msg->value.data = (uint8_t *)malloc(data->datasize); // Allocate memory for the data
                if (msg->value.data != NULL) {
                    memcpy(msg->value.data, data->data, data->datasize); // Copy the data
                    stats->num_operations++;
                    stats->time += get_time_micros()-init_time;
                } 
                else {
                    msg->opcode = MESSAGE_T__OPCODE__OP_ERROR;
                    msg->c_type = MESSAGE_T__C_TYPE__CT_NONE;
                }
            } 
        }

        else{
            msg->opcode = MESSAGE_T__OPCODE__OP_ERROR;
            msg->c_type = MESSAGE_T__C_TYPE__CT_NONE; 
        }
    }

    else if(msg->opcode == MESSAGE_T__OPCODE__OP_DEL && msg->c_type == MESSAGE_T__C_TYPE__CT_KEY){

        unsigned long init_time = get_time_micros();

        if (msg->key != NULL){

            if(table_remove(table,msg->key) == 0){
                msg->opcode = MESSAGE_T__OPCODE__OP_DEL+1;
                msg->c_type = MESSAGE_T__C_TYPE__CT_NONE;
                stats->num_operations++;
                stats->time += get_time_micros()-init_time;
            }
            else if(table_remove(table,msg->key) == -1){
                msg->opcode = MESSAGE_T__OPCODE__OP_ERROR;
                msg->c_type = MESSAGE_T__C_TYPE__CT_NONE;  
            }
            
        }
       
        else{
            msg->opcode = MESSAGE_T__OPCODE__OP_ERROR;
            msg->c_type = MESSAGE_T__C_TYPE__CT_NONE;  
        }
    }

    else if(msg->opcode == MESSAGE_T__OPCODE__OP_SIZE && msg->c_type == MESSAGE_T__C_TYPE__CT_NONE){

        unsigned long init_time = get_time_micros();

        int size = table_size(table);

        if(size > 0){

            msg->opcode = MESSAGE_T__OPCODE__OP_SIZE+1;
            msg->c_type = MESSAGE_T__C_TYPE__CT_RESULT;
            msg->result = size;
            stats->num_operations++;
            stats->time += get_time_micros()-init_time;
        }

        else{
            msg->opcode = MESSAGE_T__OPCODE__OP_ERROR;
            msg->c_type = MESSAGE_T__C_TYPE__CT_NONE;  
        }
    }
    
    else if(msg->opcode == MESSAGE_T__OPCODE__OP_GETKEYS && msg->c_type == MESSAGE_T__C_TYPE__CT_NONE){

        unsigned long init_time = get_time_micros();

        char **table_keys = table_get_keys(table);

        if(table_keys != NULL){

            char **keys = malloc(table_size(table) * sizeof(char*));
            if (keys == NULL)
            {
                table_free_keys(table_keys);
                msg->opcode = MESSAGE_T__OPCODE__OP_ERROR;
                msg->c_type = MESSAGE_T__C_TYPE__CT_NONE;
                return 0;
            }
            
            for (size_t i = 0; i < table_size(table); i++)
            {
                keys[i] = strdup(table_keys[i]);
                if (keys[i] == NULL) {
                    table_free_keys(table_keys);
                    free(keys);
                    msg->opcode = MESSAGE_T__OPCODE__OP_ERROR;
                    msg->c_type = MESSAGE_T__C_TYPE__CT_NONE;
                    return 0;
                }
            }
            
            table_free_keys(table_keys);
            msg->opcode = MESSAGE_T__OPCODE__OP_GETKEYS+1;
            msg->c_type = MESSAGE_T__C_TYPE__CT_KEYS;
            msg->keys = keys;
            msg->n_keys = table_size(table);
            stats->num_operations++;
            stats->time += get_time_micros()-init_time;
        }

        else{
            msg->opcode = MESSAGE_T__OPCODE__OP_ERROR;
            msg->c_type = MESSAGE_T__C_TYPE__CT_NONE;  
        }
    }

    else if(msg->opcode == MESSAGE_T__OPCODE__OP_GETTABLE && msg->c_type == MESSAGE_T__C_TYPE__CT_NONE){

        unsigned long init_time = get_time_micros();

        char **table_keys = table_get_keys(table);

        if(table_keys != NULL){

            EntryT **entries = malloc(table_size(table) * sizeof(EntryT*));
            if (entries == NULL) {
                msg->opcode = MESSAGE_T__OPCODE__OP_GETTABLE+1;
                msg->c_type = MESSAGE_T__C_TYPE__CT_TABLE;
                return 0;
            }

            for(int i = 0; i < table_size(table); i++){

                struct data_t *data = table_get(table, table_keys[i]);

                if(data!= NULL){
                    void *content = malloc(data->datasize);
                    if (content == NULL) {
                        data_destroy(data);
                        table_free_keys(table_keys);
                        msg->opcode = MESSAGE_T__OPCODE__OP_GETTABLE+1;
                        msg->c_type = MESSAGE_T__C_TYPE__CT_TABLE;
                        return 0;
                    }
                    memcpy(content, data->data, data->datasize);

                    EntryT *entry = malloc(sizeof(EntryT));
                    if (entry == NULL) {
                        free(content);
                        data_destroy(data);
                        table_free_keys(table_keys);
                        msg->opcode = MESSAGE_T__OPCODE__OP_GETTABLE+1;
                        msg->c_type = MESSAGE_T__C_TYPE__CT_TABLE;
                        return 0;
                    }

                    entry_t__init(entry);
                    entry->key = strdup(table_keys[i]);
                    entry->value.data = content;
                    entry->value.len = data->datasize;
                    if (entry->key == NULL) {
                        entry_t__free_unpacked(entry, NULL);
                        free(content);
                        data_destroy(data);
                        table_free_keys(table_keys);
                        msg->opcode = MESSAGE_T__OPCODE__OP_GETTABLE+1;
                        msg->c_type = MESSAGE_T__C_TYPE__CT_TABLE;
                        return 0;
                    }
                    data_destroy(data);
                    entries[i] = entry;
                    
                } else {
                    table_free_keys(table_keys);
                    msg->opcode = MESSAGE_T__OPCODE__OP_GETTABLE+1;
                    msg->c_type = MESSAGE_T__C_TYPE__CT_TABLE;
                    return 0;
                }

            }
            table_free_keys(table_keys);
            msg->n_entries = table_size(table);
            msg->entries = entries;
            msg->opcode = MESSAGE_T__OPCODE__OP_GETTABLE+1;
            msg->c_type = MESSAGE_T__C_TYPE__CT_TABLE;
            stats->num_operations++;
            stats->time += get_time_micros()-init_time;
        }

        else{
            msg->opcode = MESSAGE_T__OPCODE__OP_ERROR;
            msg->c_type = MESSAGE_T__C_TYPE__CT_NONE;  
        }
    }
    else if(msg->opcode == MESSAGE_T__OPCODE__OP_STATS && msg->c_type == MESSAGE_T__C_TYPE__CT_NONE){
        
        StatisticsT *send_stats = (StatisticsT *)malloc(sizeof(StatisticsT));

        if(send_stats ==NULL){
            msg->opcode = MESSAGE_T__OPCODE__OP_ERROR;
            msg->c_type = MESSAGE_T__C_TYPE__CT_NONE;
            return -1;
        }
        else{
            statistics_t__init(send_stats);
            msg->opcode = MESSAGE_T__OPCODE__OP_STATS+1;
            msg->c_type = MESSAGE_T__C_TYPE__CT_STATS;
            msg->stats = send_stats;
            msg->stats->num_operations = stats->num_operations;
            msg->stats->time = stats->time;
            msg->stats->num_clients = stats->num_clients;
            
        }  
    }

    return 0;
    
}

