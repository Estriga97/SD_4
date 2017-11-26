//Nomes: António Estriga, Diogo Ferreira, Francisco Caeiro
//Numeros: 47839, 47840, 47823
//Grupo: 40
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>    ola


#include "entry.h"
#include "entry-private.h"
#include "data.h"


void entry_destroy(struct entry_t *entry){
	
	if(entry != NULL){
		free(entry -> key);
		data_destroy(entry -> value);}
		
}

/* Função que inicializa os membros de uma entrada na tabela com
   o valor NULL.
 */
void entry_initialize(struct entry_t *entry) {
	if(entry!= NULL){
		entry -> key = NULL;
		entry -> value = NULL;
		entry -> next = NULL;}
}


/* Função que duplica um par {chave, valor}.
 */
struct entry_t *entry_dup(struct entry_t *entry){
    
	struct entry_t* result = NULL;

	if(entry!= NULL){
    
	result = (struct entry_t*) malloc(sizeof(struct entry_t));

	result -> key = (char*) malloc(strlen(entry -> key)+1);
	memcpy(result -> key, entry -> key, strlen(entry -> key)+1);
	
	result -> value = data_dup(entry -> value);
    
	result -> next = entry -> next;

	}

	return result;
}

/* Função que devolve o size da entry.
 */
 
 /*int* entry_size(struct entry_t* entry) {
	 int[] res = {strlen(entry -> key), entry -> value -> datasize};
	 return res;
 }*/


