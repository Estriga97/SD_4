#ifndef _NETWORK_SERVER_H
#define _NETWORK_SERVER_H

#include "primary_backup-private.h"
#include "inet.h"
struct message_t *network_send_receive(struct server_t *server, struct message_t *msg);

int server_connect(struct server_t* sec_serv);

#endif