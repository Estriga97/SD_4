#ifndef _TABLE_H
#define _TABLE_H

#include "entry.h"

struct table_t; /* A definir pelo grupo em table-private.h */

/* Função para criar/inicializar uma nova tabela hash, com n  
 * linhas(n = módulo da função hash)
 */
struct table_t *table_create(int n);

/* Libertar toda a memória ocupada por uma tabela.
 */
void table_destroy(struct table_t *table);

/* Função para adicionar um par chave-valor na tabela. 
 * Os dados de entrada desta função deverão ser copiados.
 * Devolve 0 (ok) ou -1 (out of memory, outros erros)
 */
int table_put(struct table_t *table, char *key, struct data_t *value);

/* Função para substituir na tabela, o valor associado à chave key. 
 * Os dados de entrada desta função deverão ser copiados.
 * Devolve 0 (OK) ou -1 (out of memory, outros erros)
 */
int table_update(struct table_t *table, char *key, struct data_t *value);

/* Função para obter da tabela o valor associado à chave key.
 * A função deve devolver uma cópia dos dados que terão de ser libertados
 * no contexto da função que chamou table_get.
 * Devolve NULL em caso de erro.
 */
struct data_t *table_get(struct table_t *table, char *key);

/* Devolve o número de elementos na tabela.
 */
int table_size(struct table_t *table);

/* Devolve um array de char * com a cópia de todas as keys da tabela,
 * e um último elemento a NULL.
 */
char **table_get_keys(struct table_t *table);

/* Liberta a memória alocada por table_get_keys().
 */
void table_free_keys(char **keys);

#endif
