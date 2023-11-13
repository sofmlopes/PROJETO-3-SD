#ifndef _MESSAGE_H
#define _MESSAGE_H /* Módulo message*/

#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>

/**
 * Função que recebe buffers de qualquer dimensão pela rede,
 * e assegura que só retorna quando o buffer for totalmente recebido
*/
int read_all(int sock, char *buf, int len);

/**
 * Função que envia buffers de qualquer dimensão pela rede,
 * e assegura que só retorna quando o buffer for totalmente enviado
*/
int write_all(int sock, char *buf, int len);

#endif