/**
 * Número do grupo: 60
 * João Vale 58159
 * Sofia Lopes 58175
 * Eduardo Santos 58185
*/

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>

#include "client_stub.h"
#include "client_stub-private.h"
#include "data.h"
#include "entry.h"
#include "network_client.h"
#include "sdmessage.pb-c.h"
#include "stats.h"

void stopexec() {
    exit(0);
}
/**
 * table-client <server>:<port>
    <server> é o endereço IP ou nome do servidor da tabela.
    <port> é o número do porto TCP onde o servidor está à espera de ligações.
*/

int main(int argc, char **argv) {

    // SIGPIPE
    signal(SIGPIPE, SIG_IGN);
    signal(SIGINT, stopexec);

    if (argc < 2) {
        fprintf(stderr, "Invalid args!\n Usage: table-client <server>:<port>\n");
        return -1;
    }

    struct rtable_t *rtable = rtable_connect(argv[1]);

    if (rtable == NULL) {
        fprintf(stderr, "Erro ao conectar-se ao servidor: Connection refused\n");
        exit(-1);
    }

    char command[2048];

    while (1) {

        printf("Command: ");

        if (fgets(command, sizeof(command), stdin) == NULL) 
            return -1;

        command[strlen(command) - 1] = '\0';  // Remove a quebra de linha

        if (strcmp(command, "quit") == 0 || strncmp(command, "q", 1) == 0) {
            printf("Bye, bye!\n");
            rtable_disconnect(rtable);
            return -1;  
        } 

        else if (strncmp(command, "put", 3) == 0 || strncmp(command, "p", 1) == 0) {

            char *key = strtok(command + 4, " ");
            char *data = strtok(NULL, "");

            if (key != NULL && data != NULL) {

                char *conteudo = strdup(data);

                struct data_t *data_entry = data_create(strlen(data), conteudo);
                if (data_entry == NULL) {
                    fprintf(stderr, "Failed to put entry.\n");
                }
                struct entry_t *entry = entry_create(strdup(key), data_entry);

                if (entry == NULL) {
                    fprintf(stderr, "Failed to put entry.\n");
                    data_destroy(data_entry);
                }
                int result = rtable_put(rtable, entry);
                if (result != 0)
                    fprintf(stderr, "Failed to put entry.\n");

                entry_destroy(entry);
            } 
            else
                fprintf(stderr, "Invalid arguments. Usage: put <key> <value>\n");
        }

        else if (strncmp(command, "get", 3) == 0 || strncmp(command, "g", 1) == 0) {

            char *key = strtok(command + 4, "");

            if (key != NULL) {

                struct data_t *data = rtable_get(rtable, key);

                if (data != NULL) {
                    char str[data->datasize + 1];
                    str[data->datasize] = '\0';
                    memcpy(str, data->data, data->datasize);
                    printf("%s\n", str);
                    data_destroy(data);
                }
                else 
                    fprintf(stderr, "Error in rtable_get or key not found\n");
            } 
            else
                fprintf(stderr, "Invalid arguments. Usage: get <key>\n");

        } 
        else if (strncmp(command, "del", 3) == 0 || strncmp(command, "d", 1) == 0) {

            char *key = strtok(command + 4, "");

            if (key != NULL) {

                if (rtable_del(rtable, key) == 0) 
                    printf("Entry removed.\n"); 
                else 
                    fprintf(stderr, "Error in rtable_del or key not found\n");
            } 
            else 
                fprintf(stderr, "Invalid arguments. Usage: del <key>\n");
        } 
        
        else if (strncmp(command, "size", 4) == 0 || strncmp(command, "s", 1) == 0) 
            printf("Table size: %d\n", rtable_size(rtable));

        else if (strncmp(command, "getkeys", 7) == 0 || strncmp(command, "k", 1) == 0) {

            char **keys = rtable_get_keys(rtable);

            if (keys != NULL) {

                for (int i = 0; keys[i] != NULL; i++) {
                    printf("%s\n", keys[i]);
                }
                rtable_free_keys(keys);
            } 
            else 
                fprintf(stderr, "Error in rtable_get_keys\n");
        } 

        else if (strncmp(command, "gettable", 8) == 0 || strncmp(command, "t", 1) == 0) {

            struct entry_t **entries = rtable_get_table(rtable);

            if (entries != NULL) {

                for (int i = 0; entries[i] != NULL; i++) {
                    char str[entries[i]->value->datasize + 1];
                    str[entries[i]->value->datasize] = '\0';
                    memcpy(str, entries[i]->value->data, entries[i]->value->datasize);
                    printf("%s :: %s\n", entries[i]->key, str);
                }

                rtable_free_entries(entries);
            } 
        } 

/*
        else if (strncmp(command, "stats", 5) == 0 || strncmp(command, "st", 2) == 0) {

            struct statistics_t *stats = rtable_stats(rtable);

            if (stats != NULL) {

                //fazer coisas

            } 
        } 
    */

        else 
            fprintf(stderr, "Invalid command.\n Usage: p[ut] <key> <value> | g[et] <key> | d[el] <key> | s[ize] | [get]k[eys] | [get]t[able] | q[uit] | st[ats] \n");
    }

    return 0;
}
