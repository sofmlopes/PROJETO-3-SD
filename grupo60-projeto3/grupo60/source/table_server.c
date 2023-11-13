/**
 * Número do grupo: 60
 * João Vale 58159
 * Sofia Lopes 58175
 * Eduardo Santos 58185
*/

#include <stdio.h>
#include <stdlib.h>

#include "network_server.h"
#include "sdmessage.pb-c.h"
#include "table_skel.h"
#include "table.h"  

/**
 * table-server <port> <n_lists>
    <port> é o número do porto TCP ao qual o servidor se deve associar (fazer bind).
    <n_lists> é o número de listas usado na criação da tabela no servidor.
*/

int main(int argc, char **argv) {

    if (argc != 3) {
        printf("Invalid arguments! \nUsage: table-server <port> <n_lists>\n");
        return -1;
    }

    int port = atoi(argv[1]);
    int n_lists = atoi(argv[2]);

    if ((port >= 0 && port<= 1023) || (port >= 49152 && port <= 65535)) {
        printf("Bad port number\n");
        return -1;
    }

    if (n_lists <=0){
        printf("Error initializing table\n");
        return -1;
    }

    struct table_t *table = table_skel_init(n_lists);

    if (table == NULL) {
        printf("Error initializing table.\n");
        return -1;
    }

    // Inicializa o servidor e entra no loop principal
    int listening_socket = network_server_init((short)port);
    
    if (listening_socket ==  -1) {
        printf("Error in bind: Address already in use \n Error initializing network");
        table_skel_destroy(table);
        return -1;
    }

    // Entra no loop principal do servidor
    if (network_main_loop(listening_socket, table) == -1) {
        printf("Error: Failed to run the server main loop.\n");
        network_server_close(listening_socket);
        table_skel_destroy(table);
        return -1;
    }

    // Limpeza: fecha o socket do servidor e destrói a tabela
    network_server_close(listening_socket);
    table_skel_destroy(table);

    return 0;
}
