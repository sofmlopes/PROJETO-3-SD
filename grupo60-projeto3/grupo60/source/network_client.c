/**
 * Número do grupo: 60
 * João Vale 58159
 * Sofia Lopes 58175
 * Eduardo Santos 58185
*/

#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "client_stub.h"
#include "client_stub-private.h"
#include "message.h"
#include "sdmessage.pb-c.h"

/* Esta função deve:
 * - Obter o endereço do servidor (struct sockaddr_in) com base na
 *   informação guardada na estrutura rtable;
 * - Estabelecer a ligação com o servidor;
 * - Guardar toda a informação necessária (e.g., descritor do socket)
 *   na estrutura rtable;
 * - Retornar 0 (OK) ou -1 (erro).
 */
int network_connect(struct rtable_t *rtable){

    int sockfd;
    struct sockaddr_in server;

    if(rtable == NULL)
        return -1;

    // Cria socket TCP
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Erro ao criar socket TCP");
        return -1;
    }

    // Preenche estrutura server para estabelecer conexão
    server.sin_family = AF_INET;
    server.sin_port = htons(rtable->server_port);
    if (inet_pton(AF_INET, rtable->server_address, &server.sin_addr) < 1) {
        printf("Erro ao converter IP\n");
        close(sockfd);
        return -1;
    }

    // Estabelece conexão com o servidor definido em server
    if (connect(sockfd,(struct sockaddr *)&server, sizeof(server)) < 0) {
        perror("Erro ao conectar-se ao servidor");
        close(sockfd);
        return -1;
    }

    // Armazena o descritor do socket na estrutura rtable
    rtable->sockfd = sockfd;

    return 0;
}

MessageT *network_send_receive(struct rtable_t *rtable, MessageT *msg) {
    
    if (rtable == NULL || rtable->sockfd < 0 || msg == NULL) {
        perror("Invalid argument or socket not initialized");
        return NULL;
    }

    // Serialize the message
    size_t len = message_t__get_packed_size(msg);
    uint8_t *buffer_send = malloc(len);
    if (buffer_send == NULL) {
        perror("Memory allocation failed");
        return NULL;
    }

    if (message_t__pack(msg, buffer_send) != len) {
        free(buffer_send);
        perror("Serialization failed");
        return NULL;
    }

    // Send the message length
    uint16_t send_len = htons(len);
    if (write_all(rtable->sockfd, (char *)&send_len, sizeof(send_len)) != sizeof(send_len)) {
        free(buffer_send);
        perror("Failed to send message length");
        return NULL;
    }

    // Send the serialized message
    if (write_all(rtable->sockfd, (char *)buffer_send, len) != len) {
        free(buffer_send);
        perror("Failed to send serialized message");
        return NULL;
    }

    free(buffer_send);

    // Receive the response length
    uint16_t receive_len;
    if (read_all(rtable->sockfd, (char *)&receive_len, sizeof(receive_len)) != sizeof(receive_len)) {
        perror("Failed to receive response length");
        return NULL;
    }

    receive_len = ntohs(receive_len);
    
    // Receive the response
    char *buffer_receive = malloc(receive_len);
    if (buffer_receive == NULL) {
        perror("Memory allocation failed for response");
        return NULL;
    }

    if (read_all(rtable->sockfd, buffer_receive, receive_len) != receive_len) {
        free(buffer_receive);
        perror("Failed to receive response data");
        return NULL;
    }

    // Deserialize the response
    MessageT* response = message_t__unpack(NULL, receive_len, (const uint8_t *)buffer_receive);
    if (response == NULL) {
        free(buffer_receive);
        perror("Deserialization of response failed");
        return NULL;
    }
    free(buffer_receive);

    return response;
}


/* Fecha a ligação estabelecida por network_connect().
 * Retorna 0 (OK) ou -1 (erro).
 */
int network_close(struct rtable_t *rtable){

    if (rtable == NULL)
        return -1;

    if (rtable->sockfd < 0)
        return -1;
        
    if (close(rtable->sockfd) == -1) 
        return -1;

    return 0;
}
