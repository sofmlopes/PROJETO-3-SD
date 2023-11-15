/**
 * Número do grupo: 60
 * João Vale 58159
 * Sofia Lopes 58175
 * Eduardo Santos 58185
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h> 
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <pthread.h>

#include "network_server.h"
#include "sdmessage.pb-c.h"
#include "message.h"
#include "table_skel.h"
#include "table.h"

struct thread_parameters {
	struct table_t *table;
	int connsockfd;
};

/* Função para preparar um socket de receção de pedidos de ligação
 * num determinado porto.
 * Retorna o descritor do socket ou -1 em caso de erro.
 */
int network_server_init(short port){

    int sockfd; 
    struct sockaddr_in server;
    int opt = 1;

    // Cria socket TCP
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0 ) {
        perror("Erro ao criar socket");
        return -1;
    }

    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
        perror("Erro ao configurar opções do socket");
        close(sockfd);
        return -1;
    }

    // Preenche estrutura server para bind
    server.sin_family = AF_INET;
    server.sin_port = htons(port); 
    server.sin_addr.s_addr = htonl(INADDR_ANY);

    // Faz bind
    if (bind(sockfd, (struct sockaddr *) &server, sizeof(server)) < 0){
        perror("Erro ao fazer bind");
        close(sockfd);
        return -1;
    };

    // Faz listen
    if (listen(sockfd, 10) < 0){
        perror("Erro ao executar listen");
        close(sockfd);
        return -1;
    };

    return sockfd;

}

void *handle_client(void *params) {

    struct thread_parameters *tp = (struct thread_parameters *) params;

    while (1) { 
                
        //Receber uma mensagem usando a função network_receive
        MessageT *msg = network_receive(tp->connsockfd);

        if (msg == NULL) {
            // perror("Erro ao receber mensagem do cliente");
            close(tp->connsockfd);
            printf("Server ready, waiting for connections\n");
            break;
        }

        //Entregar a mensagem de-serializada ao skeleton para ser processada na tabela table
        int result = invoke(msg, tp->table);

        //Esperar a resposta do skeleton
        if (result == -1) {
            perror("Erro ao processar mensagem do cliente");
            close(tp->connsockfd);
            printf("Server ready, waiting for connections\n");
            break;
        }

        //Enviar a resposta ao cliente usando a função network_send
        if (network_send(tp->connsockfd, msg) == -1) {
            perror("Erro ao enviar resposta ao cliente");
            close(tp->connsockfd);
            printf("Server ready, waiting for connections\n");
            break;
        }

    }
}


/* A função network_main_loop() deve:
 * - Aceitar uma conexão de um cliente;
 * - Receber uma mensagem usando a função network_receive;
 * - Entregar a mensagem de-serializada ao skeleton para ser processada
     na tabela table;
 * - Esperar a resposta do skeleton;
 * - Enviar a resposta ao cliente usando a função network_send.
 * A função não deve retornar, a menos que ocorra algum erro. Nesse
 * caso retorna -1.
 */
int network_main_loop(int listening_socket, struct table_t *table){

    int connsockfd;
    struct sockaddr_in client;
    socklen_t size_client = sizeof(struct sockaddr_in);

    printf("Server ready, waiting for connections\n");

        //Aceitar uma conexão de um cliente
        while ((connsockfd = accept(listening_socket,(struct sockaddr *) &client, &size_client)) != -1){
                pthread_t thread_id;
                int *i = malloc(sizeof(int));
                *i = connsockfd;

                if(pthread_create(&thread_id, NULL, &handle_client, i) < 0){
                    perror("could not create thread");
                    free(i);
                    return -1;
                }
                pthread_detach(thread_id);

                printf("Client connection established\n");

    }

    // Fecha o socket do cliente
    close(connsockfd);

    // Se o loop terminar (o que normalmente não deveria acontecer)
    return -1;
    
}

/* A função network_receive() deve:
 * - Ler os bytes da rede, a partir do client_socket indicado;
 * - De-serializar estes bytes e construir a mensagem com o pedido,
 *   reservando a memória necessária para a estrutura MessageT.
 * Retorna a mensagem com o pedido ou NULL em caso de erro.
 */
MessageT *network_receive(int client_socket) {

    // Passo 1: Ler os 2 bytes iniciais (tamanho da mensagem)
    uint16_t message_size;
    if (read_all(client_socket, (char *)&message_size, sizeof(message_size)) != sizeof(message_size)) {
        // perror("Erro ao receber tamanho da mensagem");
        return NULL;
    }

    message_size = ntohs(message_size); // Converte de rede para host byte order

    // Passo 2: Alocar memória para a mensagem serializada

    char *serialized_message = (char *)malloc(message_size);
    if (serialized_message == NULL) {
        perror("Erro de alocação de memória");
        return NULL;
    }

    // Passo 3: Ler os bytes restantes da mensagem serializada

    ssize_t bytes_received = read_all(client_socket, (char *)serialized_message, message_size);
    if (bytes_received != message_size) {
        perror("Erro ao receber mensagem serializada");
        free(serialized_message);
        return NULL;
    }

    // Passo 4: Deserializar a mensagem

    MessageT *msg = message_t__unpack(NULL, (size_t)message_size, (const uint8_t *)serialized_message);

    // Libertar a memória alocada para a mensagem serializada

    free(serialized_message);

    if (msg == NULL) {
        perror("Erro ao deserializar a mensagem");
        return NULL;
    }

    // Passo 5: Retornar a mensagem
    return msg;
}


/* A função network_send() deve:
 * - Serializar a mensagem de resposta contida em msg;
 * - Enviar a mensagem serializada, através do client_socket.
 * Retorna 0 (OK) ou -1 em caso de erro.
 */
int network_send(int client_socket, MessageT *msg) {
    if (client_socket == 0 || msg == NULL) 
        return -1;

    size_t len = message_t__get_packed_size(msg);
    ssize_t bytes_sent;

    // Convert the length to network byte order (big-endian)
    uint16_t send_len = htons(len);

    // Send the length of the message first
    bytes_sent = write_all(client_socket, &send_len, sizeof(send_len));
    if (bytes_sent != sizeof(send_len)) {
        // Handle error if unable to send length
        return -1;
    }

    // Serialize the message
    uint8_t *buf = malloc(len);
    if (buf == NULL) 
        return -1;

    if (message_t__pack(msg, buf) == -1) {
        free(buf);
        return -1;
    }

    // Send the serialized message
    bytes_sent = write_all(client_socket, buf, len);
    free(buf); // Free the serialized buffer

    if (bytes_sent != len) {
        // Handle error if unable to send serialized message
        return -1;
    }

    return 0;
}



/* Liberta os recursos alocados por network_server_init(), nomeadamente
 * fechando o socket passado como argumento.
 * Retorna 0 (OK) ou -1 em caso de erro.
 */
int network_server_close(int socket){

    if (socket < 0) 
        return -1;
        
    if (close(socket) == -1) {
        perror("Erro ao fechar o socket do servidor");
        return -1;
    }

    return 0;

}


