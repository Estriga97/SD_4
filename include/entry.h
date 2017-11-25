#ifndef _ENTRY_H
#define _ENTRY_H

#include "data.h"

/* Esta estrutura define o par {chave, valor} para a tabela
 */
struct entry_t {
	char *key;	/* string, (char* terminado por '\0') */
	struct data_t *value; /* Bloco de dados */
	struct entry_t *next; /* Resolução de colisões */
};

/* Função que inicializa os membros de uma entrada na tabela com
   o valor NULL.
 */
void entry_initialize(struct entry_t *entry);

/* Função que duplica um par {chave, valor}.
 */
struct entry_t *entry_dup(struct entry_t *entry);

#endif
