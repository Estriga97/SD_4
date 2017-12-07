#ifndef _PRIMARY_BACKUP_PRIVATE_H
#define _PRIMARY_BACKUP_PRIVATE_H

#include "primary_backup.h"
#include "data.h"


struct server_t {
    char* ip_port;
    int state; // 0 = DOWN; 1 = UP
    int socket;
};

int rtables_sz_tbles(struct server_t *server,char** lst_tbls, int size);

int rtables_ack(struct server_t *server);

int rtables_put(struct server_t *server,int table_num, char *key, struct data_t *value);

int rtables_update(struct server_t *server,int table_num, char *key, struct data_t *value);

#endif