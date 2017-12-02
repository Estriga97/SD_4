#ifndef _NETWORK_CLIENT_PRIVATE_H
#define _NETWORK_CLIENT_PRIVATE_H

#include "inet.h"
#include "network_client.h"

#define RETRY_TIME 5

struct server_t{
	int socket;
	int server_type; //1 se for o principal, 0 se for o secundario
	char *ip_port_primario; //ip e port do servidor primario
	char *ip_port_secundario; //ip e port do servidor secundario
	/* Atributos importantes para interagir com o servidor, */
	/* tanto antes da ligação estabelecida, como depois.    */
};

/* Função que garante o envio de len bytes armazenados em buf,
   através da socket sock.
*/
int write_all(int sock, char *buf, int len);

/* Função que garante a receção de len bytes através da socket sock,
   armazenando-os em buf.
*/
int read_all(int sock, char *buf, int len);

#endif
