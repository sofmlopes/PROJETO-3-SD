/**
 * Número do grupo: 60
 * João Vale 58159
 * Sofia Lopes 58175
 * Eduardo Santos 58185
*/

#include "sdmessage.pb-c.h"
#include "message.h"

/**
 * Função que recebe buffers de qualquer dimensão pela rede,
 * e assegura que só retorna quando o buffer for totalmente recebido
*/
int read_all(int sock, char *buf, int len){

	int bufsize = len;

	while(len>0) {

		int res = read(sock, buf, len);

		if(res == 0)
			return 0;

		if(res<=0) {

			if(errno==EINTR) continue;
			perror("Read failed:");
			return res;
		}

		buf += res;
		len -= res;
	}

	return bufsize;
}

/**
 * Função que envia buffers de qualquer dimensão pela rede,
 * e assegura que só retorna quando o buffer for totalmente enviado
*/
int write_all(int sock, char *buf, int len) {

    int bufsize = len;

    while(len>0) {

        int res = write(sock, buf, len);

        if(res<0) {

            if(errno==EINTR) continue;
            perror("write failed:");
            return res; /* Error != EINTR */
        }

        if(res==0) return res; /* Socket was closed */
        buf += res;
        len -= res;
    }

    return bufsize;
}
