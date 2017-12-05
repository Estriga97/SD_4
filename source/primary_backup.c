
#include "primary_backup-private.h"
#include "table_server.h"

/* Função usada para um servidor avisar o servidor “server” de que
* já acordou. Retorna 0 em caso de sucesso, -1 em caso de insucesso
*/
int hello(struct server_t *server) {
    return server_connect(server);
}

/* Pede atualizacao de estado ao server.
* Retorna 0 em caso de sucesso e -1 em caso de insucesso.
*/
int update_state(struct server_t *server) {

    int res;
    if((res = hello(server)) < 0 ) {
        sleep(5);
        res = hello(server);
        if(res < 0)
            return -1;
    }
    int* ack;
        if((ack = (int*) malloc(sizeof(int))) == NULL) {
            //*
        }
    *ack = 0;

    while(!*ack) {
        if((res = network_receive_send(o_server->socket, ack)) < 0)
            return -1;
    }
    return res;

}
