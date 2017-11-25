#ifndef _DATA_H
#define _DATA_H

/* Estrutura que define os dados.  
 */
struct data_t {
	int datasize; /* Tamanho do bloco de dados */
	void *data;   /* Conteúdo arbitrário */
};

/* Função que cria um novo elemento de dados data_t e reserva a memória
 * necessária, especificada pelo parâmetro size 
 */
struct data_t *data_create(int size);

/* Função idêntica à anterior, mas que inicializa os dados de acordo com
 * o parâmetro data.
 */
struct data_t *data_create2(int size, void * data); 

/* Função que destrói um bloco de dados e liberta toda a memória.
 */
void data_destroy(struct data_t *data);

/* Função que duplica uma estrutura data_t.
 */
struct data_t *data_dup(struct data_t *data);

#endif
