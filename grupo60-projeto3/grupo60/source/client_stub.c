/**
 * Número do grupo: 60
 * João Vale 58159
 * Sofia Lopes 58175
 * Eduardo Santos 58185
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "client_stub-private.h"
#include "client_stub.h"
#include "data.h"
#include "entry.h"
#include "message.h"
#include "network_client.h"
#include "sdmessage.pb-c.h"
#include "stats.h"


/* Função para estabelecer uma associação entre o cliente e o servidor, 
 * em que address_port é uma string no formato <hostname>:<port>.
 * Retorna a estrutura rtable preenchida, ou NULL em caso de erro.
 */
struct rtable_t *rtable_connect(char *address_port){

    if (address_port == NULL)
	   return NULL;

    struct rtable_t *new_rtable;
    new_rtable= (struct rtable_t*) malloc(sizeof(struct rtable_t));

    if (new_rtable== NULL)
        return NULL;
    
    char *hostname = strtok(address_port, ":");
    char *port = strtok(NULL, "");

    if (hostname == NULL || port == NULL) {
        free(new_rtable);
        return NULL;
    }

    new_rtable->server_address = hostname;
    new_rtable->server_port = atoi(port);

    if (network_connect(new_rtable) != 0) {
        free(new_rtable);
        return NULL;
    }

    return new_rtable;

}

/* Termina a associação entre o cliente e o servidor, fechando a 
 * ligação com o servidor e libertando toda a memória local.
 * Retorna 0 se tudo correr bem, ou -1 em caso de erro.
 */
int rtable_disconnect(struct rtable_t *rtable){

    if (rtable == NULL)
        return -1;

    if(network_close(rtable)==-1)
        return -1;

    free(rtable);

    return 0;
}

/* Função para adicionar um elemento na tabela.
 * Se a key já existe, vai substituir essa entrada pelos novos dados.
 * Retorna 0 (OK, em adição/substituição), ou -1 (erro).
 */
int rtable_put(struct rtable_t *rtable, struct entry_t *entry) {
    
    if (rtable == NULL || entry == NULL)
        return -1;

    if (entry->key == NULL || entry->value == NULL)
        return -1;

    if (entry->value->datasize < 0 )
        return -1;

    MessageT *msg_request = (MessageT *)malloc(sizeof(MessageT));

    if (msg_request == NULL)
        return -1;
        
    message_t__init(msg_request);
    msg_request->opcode = MESSAGE_T__OPCODE__OP_PUT;
    msg_request->c_type = MESSAGE_T__C_TYPE__CT_ENTRY;

    EntryT *entry_request = (EntryT *)malloc(sizeof(EntryT));

    if (entry_request == NULL){
        free(msg_request);
        return -1;
    }

    entry_t__init(entry_request);

    entry_request->key = entry->key;

    entry_request->value.data = entry->value->data;
    entry_request->value.len = entry->value->datasize;
    msg_request->entry = entry_request;
    
    MessageT *msg_response = network_send_receive(rtable, msg_request);

    free(entry_request);
    free(msg_request);
    
    if(msg_response == NULL)
        return -1;

    if (msg_response->opcode != MESSAGE_T__OPCODE__OP_PUT + 1) {
        message_t__free_unpacked(msg_response, NULL);
        return -1;
    }

    message_t__free_unpacked(msg_response, NULL);
    
    return 0;
}


/* Retorna o elemento da tabela com chave key, ou NULL caso não exista
 * ou se ocorrer algum erro.
 */
struct data_t *rtable_get(struct rtable_t *rtable, char *key){

    if(rtable == NULL || key ==NULL)
        return NULL;

    MessageT *msg_request = (MessageT *)malloc(sizeof(MessageT));

    if (msg_request == NULL)
        return NULL;

    message_t__init(msg_request);
    msg_request->opcode = MESSAGE_T__OPCODE__OP_GET;
    msg_request->c_type = MESSAGE_T__C_TYPE__CT_KEY;
    msg_request->key = key;

    if(msg_request->key == NULL){
        free(msg_request);
        return NULL;
    }
    
    MessageT *msg_response = network_send_receive(rtable, msg_request);

    free(msg_request);

    if(msg_response == NULL)
        return NULL;

    if(msg_response->opcode != MESSAGE_T__OPCODE__OP_GET + 1 || msg_response->c_type != MESSAGE_T__C_TYPE__CT_VALUE){
        message_t__free_unpacked(msg_response,NULL);
        return NULL;
    }
        
    if (msg_response->value.len < 0 && msg_response->value.data == NULL) {
        message_t__free_unpacked(msg_response,NULL);
        return NULL;
    }

    void *conteudo = malloc(msg_response->value.len);
    if (conteudo == NULL) {
        message_t__free_unpacked(msg_response,NULL);
        return NULL;
    }
    memcpy(conteudo, msg_response->value.data, msg_response->value.len);

    struct data_t *data = data_create(msg_response->value.len, conteudo);
    if (data != NULL) {
        message_t__free_unpacked(msg_response,NULL);
        return data; 
    }
    message_t__free_unpacked(msg_response,NULL);
    
    return NULL;

}

/* Função para remover um elemento da tabela. Vai libertar 
 * toda a memoria alocada na respetiva operação rtable_put().
 * Retorna 0 (OK), ou -1 (chave não encontrada ou erro).
 */
int rtable_del(struct rtable_t *rtable, char *key){

    if (rtable == NULL || key == NULL) 
        return -1;

    MessageT *msg_request = (MessageT *)malloc(sizeof(MessageT));

    if (msg_request == NULL)
        return -1;

    message_t__init(msg_request);
    msg_request->opcode = MESSAGE_T__OPCODE__OP_DEL;
    msg_request->c_type = MESSAGE_T__C_TYPE__CT_KEY;
    msg_request->key = key;
        
    if (msg_request->key == NULL) {
        free(msg_request);
        return -1;
    }
    
    MessageT *msg_response = network_send_receive(rtable, msg_request);

    if (msg_response == NULL) {
        message_t__free_unpacked(msg_response, NULL);
        return -1;
    }
        
    if (msg_response->opcode != MESSAGE_T__OPCODE__OP_DEL+1 ||
        msg_response->c_type != MESSAGE_T__C_TYPE__CT_NONE) {
        message_t__free_unpacked(msg_response, NULL);
        return -1;
    }

    message_t__free_unpacked(msg_response, NULL);

    return 0;
}

/* Retorna o número de elementos contidos na tabela ou -1 em caso de erro.
 */
int rtable_size(struct rtable_t *rtable){

    if (rtable == NULL) 
        return -1;

    MessageT *msg_request = (MessageT *)malloc(sizeof(MessageT));

    if (msg_request == NULL) 
        return -1;

    message_t__init(msg_request);
    msg_request->opcode = MESSAGE_T__OPCODE__OP_SIZE;
    msg_request->c_type = MESSAGE_T__C_TYPE__CT_NONE;
    MessageT *msg_response = network_send_receive(rtable, msg_request);

    free(msg_request);

    if (msg_response == NULL)
        return -1;
    
    if (msg_response->opcode != MESSAGE_T__OPCODE__OP_SIZE + 1) {
        message_t__free_unpacked(msg_response, NULL);
        return -1;
    }

    int result = msg_response->result;

    message_t__free_unpacked(msg_response, NULL);

    return result;
    
}

/* Retorna um array de char* com a cópia de todas as keys da tabela,
 * colocando um último elemento do array a NULL.
 * Retorna NULL em caso de erro.
 */
char **rtable_get_keys(struct rtable_t *rtable){

    if (rtable == NULL)
        return NULL;

    MessageT *msg_request = (MessageT *)malloc(sizeof(MessageT));

    if (msg_request == NULL)
        return NULL;

    message_t__init(msg_request);
    msg_request->opcode = MESSAGE_T__OPCODE__OP_GETKEYS;
    msg_request->c_type = MESSAGE_T__C_TYPE__CT_NONE;

    MessageT *msg_response = network_send_receive(rtable, msg_request);

    free(msg_request);

    if(msg_response == NULL) 
        return NULL;

    if (msg_response->opcode != MESSAGE_T__OPCODE__OP_GETKEYS + 1 ||
        msg_response->c_type != MESSAGE_T__C_TYPE__CT_KEYS) {
        message_t__free_unpacked(msg_response, NULL);
        return NULL;
    }

    char **keys = (char **)malloc((msg_response->n_keys + 1) * sizeof(char *));

    if (keys == NULL) {
        message_t__free_unpacked(msg_response, NULL);
        return NULL;
    }

    for (int i = 0; i < msg_response->n_keys; i++) {
        keys[i] = strdup(msg_response->keys[i]);
        if (keys[i] == NULL) {
            for (int j = 0; j < i; j++) {
                free(keys[j]);
            }
            message_t__free_unpacked(msg_response, NULL);
            free(keys);
            return NULL;
        }
    }

    keys[msg_response->n_keys] = NULL;

    message_t__free_unpacked(msg_response, NULL);

    return keys;

}

/* Liberta a memória alocada por rtable_get_keys().
 */
void rtable_free_keys(char **keys){

    if (keys == NULL) 
        return;

    for (int i = 0; keys[i] != NULL; i++)
        free(keys[i]); 

    free(keys); 
}

/* Retorna um array de entry_t* com todo o conteúdo da tabela, colocando
 * um último elemento do array a NULL. Retorna NULL em caso de erro.
 */
struct entry_t **rtable_get_table(struct rtable_t *rtable){

    if (rtable == NULL) 
        return NULL;

    MessageT *msg_request = (MessageT *)malloc(sizeof(MessageT));

    if (msg_request == NULL)
        return NULL;

    message_t__init(msg_request);
    msg_request->opcode = MESSAGE_T__OPCODE__OP_GETTABLE;
    msg_request->c_type = MESSAGE_T__C_TYPE__CT_NONE;

    MessageT *msg_response = network_send_receive(rtable, msg_request);

    free(msg_request);

    if (msg_response == NULL)
        return NULL;

    if (msg_response->opcode != MESSAGE_T__OPCODE__OP_GETTABLE+ 1 ||
        msg_response->c_type != MESSAGE_T__C_TYPE__CT_TABLE) {
        message_t__free_unpacked(msg_response, NULL);
        return NULL;
    }
    
    struct entry_t **entries = (struct entry_t **)malloc((msg_response->n_entries + 1) * sizeof(struct entry_t *));
    
    if (entries == NULL) {
        message_t__free_unpacked(msg_response, NULL);
        return NULL;
    }

    for (size_t i = 0; i < msg_response->n_entries; i++) {
        // Alocar espaço para o conteudo e copia-lo
        void *conteudo = malloc(msg_response->entries[i]->value.len);
        if (conteudo == NULL) {
            for (int j = i - 1; j >= 0; j--)
                entry_destroy(entries[j]);
            message_t__free_unpacked(msg_response, NULL);
            free(entries);
        }
        memcpy(conteudo, msg_response->entries[i]->value.data, msg_response->entries[i]->value.len);

        // Criar estrutura data_t
        struct data_t *data = data_create(msg_response->entries[i]->value.len, conteudo);
        if (data == NULL) {
            free(conteudo);
            for (int j = i - 1; j >= 0; j--)
                entry_destroy(entries[j]);
            message_t__free_unpacked(msg_response, NULL);
            free(entries);
        }

        // Criar estrutura entry_t
        struct entry_t *entry = entry_create(strdup(msg_response->entries[i]->key), data);
        if (entry == NULL) {
            data_destroy(data);
            for (int j = i - 1; j >= 0; j--)
                entry_destroy(entries[j]);
            message_t__free_unpacked(msg_response, NULL);
            free(entries);
        }

        // Colocar entry na array de entries
        entries[i] = entry;
    }

    entries[msg_response->n_entries] = NULL;

    message_t__free_unpacked(msg_response, NULL);

    return entries;
}

/* Liberta a memória alocada por rtable_get_table().
 */
void rtable_free_entries(struct entry_t **entries){

    if (entries == NULL)
        return;

    for (int i = 0; entries[i] != NULL; i++) {
        entry_destroy(entries[i]); 
    }

    free(entries); 
}

/* Obtém as estatísticas do servidor. 
*/
struct statistics_t *rtable_stats(struct rtable_t *rtable){

    if (rtable == NULL) 
        return NULL;

    MessageT *msg_request = (MessageT *)malloc(sizeof(MessageT));

    if (msg_request == NULL) 
        return NULL;

    message_t__init(msg_request);
    msg_request->opcode = MESSAGE_T__OPCODE__OP_STATS;
    msg_request->c_type = MESSAGE_T__C_TYPE__CT_NONE;

    MessageT *msg_response = network_send_receive(rtable, msg_request);

    free(msg_request);

    if (msg_response == NULL)
        return NULL;
    
    if (msg_response->opcode != MESSAGE_T__OPCODE__OP_STATS + 1 ||
        msg_response->c_type != MESSAGE_T__C_TYPE__CT_STATS) {
        message_t__free_unpacked(msg_response, NULL);
        return NULL;
    }

    struct statistics_t *stats = stats_create();
    
    if (stats == NULL) {
        message_t__free_unpacked(msg_response, NULL);
        stats_destroy(stats);
        return NULL;
    }

    stats->num_clients = msg_response->stats->num_clients;
    stats->num_operations = msg_response->stats->num_operations;
    stats->time = msg_response->stats->time;
    message_t__free_unpacked(msg_response, NULL);

    return stats;
}



