//Nomes: António Estriga, Diogo Ferreira, Francisco Caeiro
//Numeros: 47839, 47840, 47823
//Grupo: 40
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "data.h"


struct data_t *data_create(int size){

	struct data_t* result = NULL;

	if(size > 0){
		result = (struct data_t*) malloc(sizeof(struct data_t));
		result -> datasize = size;
		result -> data = malloc(size);}
	

	return result;
}

struct data_t *data_create2(int size, void * data){
	struct data_t* result= NULL;

	if(size > 0 && data != NULL){
		result = data_create(size);
		memcpy(result -> data, data, size);}
	
	return result;


}

void data_destroy(struct data_t *data){ 
    if(data!= NULL){
	    free(data-> data);
		free(data);}

}

struct data_t* data_dup(struct data_t *data){

    struct data_t* data_result = NULL;

	if(data!= NULL){
		data_result = data_create2(data -> datasize, data -> data);}

	return data_result;

}