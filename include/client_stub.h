#ifndef _CLIENT_STUB_H
#define _CLIENT_STUB_H

#include "data.h"

/* Remote table. A definir pelo grupo em client_stub-private.h 
 */
struct rtables_t; 

/* Fun��o para estabelecer uma associa��o entre o cliente e um conjunto de
 * tabelas remotas num servidor.
 * Os alunos dever�o implementar uma forma de descobrir quantas tabelas
 * existem no servidor.
 * address_port � uma string no formato <hostname>:<port>.
 * retorna NULL em caso de erro .
 */
struct rtables_t *rtables_bind(const char *address_port);

/* Termina a associa��o entre o cliente e um conjunto de tabelas remotas, e
 * liberta toda a mem�ria local. 
 * Retorna 0 se tudo correr bem e -1 em caso de erro.
 */
int rtables_unbind(struct rtables_t *rtables);

/* Fun��o para adicionar um par chave valor numa tabela remota.
 * Devolve 0 (ok) ou -1 (problemas).
 */
int rtables_put(struct rtables_t *rtables, char *key, struct data_t *value);

/* Fun��o para substituir na tabela remota, o valor associado � chave key.
 * Devolve 0 (OK) ou -1 em caso de erros.
 */
int rtables_update(struct rtables_t *rtables, char *key, struct data_t *value);

/* Fun��o para obter da tabela remota o valor associado � chave key.
 * Devolve NULL em caso de erro.
 */
struct data_t *rtables_get(struct rtables_t *tables, char *key);

/* Devolve n�mero de pares chave/valor na tabela remota.
    -1 se deu erro, ou o size da table se nao der erro
 */
int rtables_size(struct rtables_t *rtables);

/* Devolve o n�mero de colis�es existentes na tabela remota.
    -1 se deu erro, ou o numero de colisoes da table se nao der erro
 */
int rtables_collisions(struct rtables_t *rtables);

/* Devolve um array de char * com a c�pia de todas as keys da
 * tabela remota, e um �ltimo elemento a NULL, ou NULL se der erro
 */
char **rtables_get_keys(struct rtables_t *rtables);

/* Liberta a mem�ria alocada por rtables_get_keys().
 */
void rtables_free_keys(char **keys);

#endif
