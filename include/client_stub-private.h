#ifndef _CLIENT_STUB_PRIVATE_H
#define _CLIENT_STUB_PRIVATE_H

#include <error.h>
#include <stdio.h>
#include <stdlib.h>

#include "network_client-private.h"
#include "client_stub.h"
#include "client_stub-private.h"
#include "table.h"
struct rtables_t{
    struct server_t * server;
    int table_num;
    int n_tables;
};

int rtables_num_tab(struct rtables_t *rtables);

#endif