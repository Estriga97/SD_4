#ifndef _TABLE_PRIVATE_H
#define _TABLE_PRIVATE_H

# include "table.h"

struct table_t {

    int collisions;
    int datasize; /* capacidade */
    int filled; /* ocupados */
    struct entry_t * entrys; /* array de entradas */
    struct entry_t * next; /* próximo espaço disponível*/
};

void print_tables(struct table_t * table);

#endif