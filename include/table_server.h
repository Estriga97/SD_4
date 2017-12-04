#ifndef _TABLE_SERVER_H
#define _TABLE_SERVER_H

#include <error.h>
#include <signal.h>
#include <poll.h>
#include <fcntl.h>
#include <errno.h>

#include "table_skel.h"
#include "inet.h"
#include "table-private.h"
#include "message-private.h"
#include "table_skel-private.h"
#include "primary_backup-private.h"

#define SOCKETS_NUMBER 7
#define FILE_PATH_1 "SD_4/ip_1"
#define FILE_PATH_2 "SD_4/ip_2"

struct thread_param_t {
	struct message_t* msg;
    struct rtables_t rtbl;
};

int rtables_sz_tbles(struct server_t *server,char** lst_tbls, int sizE);

int rtables_ack(struct server_t *server);

struct message_t *network_send_receive(struct server_t *server, struct message_t *msg);

int rtables_put(struct rtables_t *rtables, char *key, struct data_t *value);

void shift(struct pollfd* connects, int i);

int make_server_socket(short port);

int network_receive_send(int sockfd);

int server_connect(struct server_t* sec_serv);