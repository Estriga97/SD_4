#ifndef _PRIMARY_BACKUP_H
#define _PRIMARY_BACKUP_H


struct server_t; 
/* Para definir em primary_backup-private.h */

/* Função usada para um servidor avisar o servidor “server” de que
* já acordou. Retorna 0 em caso de sucesso, -1 em caso de insucesso
*/
int hello(server_t *server);

/* Pede atualizacao de estado ao server.
* Retorna 0 em caso de sucesso e -1 em caso de insucesso.
*/
int update_state(server_t *server);

#endif