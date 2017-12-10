#ifndef _TABLE_SERVER_H
#define _TABLE_SERVER_H

#include <pthread.h>
#include <error.h>
#include <signal.h>
#include <poll.h>
#include <fcntl.h>
#include <errno.h>

#include "inet.h"
#include "table-private.h"
#include "message-private.h"
#include "table_skel-private.h"
#include "primary_backup-private.h"
#include "network_server.h"

#define SOCKETS_NUMBER 7
#define LISTENING_SOCKET 0
#define STDIN_SOCKET 1
#define MAX_READ 81
#define FILE_PATH_1 "ip_1"
#define FILE_PATH_2 "ip_2"

struct thread_param_t {
	struct message_t* msg;
    struct server_t* server;
    int table_num;
};

void shift(struct pollfd* connects, int i);

int make_server_socket(short port);

int network_receive_send(int sockfd, int* ack);

void* pthread_main(void* params);

#endif