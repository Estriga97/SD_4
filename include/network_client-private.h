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

/* Troca o servidor ativo
*/
int switch_server(struct server_t *server);

#endif
