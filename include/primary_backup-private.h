#ifndef _PRIMARY_BACKUP_PRIVATE_H
#define _PRIMARY_BACKUP_PRIVATE_H

#include "primary_backup.h"

struct server_t {
    char* ip_port;
    int state; // 0 = DOWN; 1 = UP
    int socket;
};

#endif