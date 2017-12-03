#ifndef _CLIENT_STUB_PRIVATE_H
#define _CLIENT_STUB_PRIVATE_H

#include "client_stub.h"

struct rtables_t{
    struct server_t * server;
    int table_num;
    int n_tables;
};

#endif