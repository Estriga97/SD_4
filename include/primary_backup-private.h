#ifndef _PRIMARY_BACKUP_H
#define _PRIMARY_BACKUP_H

#include "primary_backup.h"

struct server_s {
    int port_sev;
    int ip_sev;
    int state; // 0 = DOWN; 1 = UP
    int socket;

};

#endif