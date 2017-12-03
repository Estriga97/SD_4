
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

int rtables_sz_tbles(struct server_t *server,char** lst_tbls, int sizE);

int rtables_ack(struct server_t *server);

struct message_t *network_send_receive(struct server_t *server, struct message_t *msg);

int rtables_put(struct rtables_t *rtables, char *key, struct data_t *value);

void shift(struct pollfd* connects, int i);

int make_server_socket(short port);

int network_receive_send(int sockfd);

int server_connect(struct server_t* sec_serv);