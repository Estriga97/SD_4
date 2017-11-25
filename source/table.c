//Nomes: António Estriga, Diogo Ferreira, Francisco Caeiro
//Numeros: 47839, 47840, 47823
//Grupo: 40
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "table-private.h"
#include "entry-private.h"
#include "entry.h"
#include "data.h"



/* Mete todas as entries a null */
void fill_entry_null(int n, struct entry_t* entrys) {

int i;
  for (i = 0; i < n; i++) {
    entry_initialize(&(entrys[i]));
  }
}

int hash(char* key, int n) {

  int sum = 0;
  int count = strlen(key);

  if(count < 5) {
    while(count) {
      sum += (int) key[count -1];
      count--;
    }
  }
  else {
    sum = (int) key[0] + key[1] + key[count -1] + key[count -2];
  }

  return sum%n;

}

void atualize_next(struct table_t *table) {

  if (table->filled == table->datasize) return;
  while (table->next->key != NULL)
    table->next--;
}

struct entry_t* get_entry( struct table_t* table, char* key) {

  int khash = hash(key, table -> datasize);
  struct entry_t* this =  &(table -> entrys[khash]);

  if(this->key == NULL)
    return NULL;

  if(this -> key!= NULL && strcmp(this -> key, key) ) {
    while(this -> next != NULL){
      if(!strcmp(this->key,key))
        return this;
    this = this-> next;
    }
    }
  if(strcmp(this->key,key))
    return NULL;
  
  return this;
  
}


struct table_t *table_create(int n) {

  struct table_t* result;
   
  if(n > 0){
    result = (struct table_t*) malloc (sizeof(struct table_t));
    result -> datasize = n;
    result -> filled = 0;
    result -> collisions = 0;
    result -> entrys = (struct entry_t*) malloc (sizeof(struct entry_t) * n);
    fill_entry_null(n, result -> entrys);
    result -> next = &(result -> entrys[n-1]);}
  else
      result = NULL;

  return result;

}

/* Libertar toda a memória ocupada por uma tabela.
*/
void table_destroy(struct table_t *table) {

  if(table != NULL){
  int i;
  for(i = 0; i < table -> datasize; i++)
    entry_destroy(&(table->entrys[i]));

  free(table->entrys);
  free(table);
  }
}

/* Função para adicionar um par chave-valor na tabela.
* Os dados de entrada desta função deverão ser copiados.
* Devolve 0 (ok) ou -1 (out of memory, outros erros)
*/
int table_put(struct table_t *table, char *key, struct data_t *value) {

  int khash = hash(key, table -> datasize);
  struct entry_t* entry = &(table -> entrys[khash]);

  if (table -> filled == table -> datasize) { // tabela cheia
    return -1;

  }
  
  if(entry->key == NULL ){//pode se adicionar logo
     entry -> key = strdup(key);
     entry -> value = data_dup (value);}

  else { // tem algo no lugar para adicionar
    if(!strcmp(entry -> key,key)) //averiguar se aquela chave não está já introduzida
       return -1;
    table ->collisions++;//colisao
    while(entry -> next != NULL){ //descobrir a proxima entry na lista ligada, para adicionar ao final 
      entry = entry -> next;
      if(!strcmp(entry -> key,key)) //averiguar se aquela chave não está já introduzida
       return -1;
    }
    entry -> next = table -> next;
    (table -> next) -> key = strdup(key);
    (table -> next) -> value = data_dup (value);

  }
  table -> filled++;

  if (table -> filled != 0) // tem mais de 1 elemento
      atualize_next(table);

  return 0;

}

/* Função para substituir na tabela, o valor associado à chave key.
* Os dados de entrada desta função deverão ser copiados.
* Devolve 0 (OK) ou -1 (out of memory, outros erros)
*/
int table_update(struct table_t *table, char *key, struct data_t *value) {

  struct entry_t* this = get_entry(table,key);
  
  if(this == NULL) 
    return -1;

  else {
    data_destroy(this -> value);
    this -> value=data_dup(value);
    return 0;
    }
  
}

/* Função para obter da tabela o valor associado à chave key.
* A função deve devolver uma cópia dos dados que terão de
* ser libertados no contexto da função que chamou table_get.
* Devolve NULL em caso de erro.
*/
struct data_t *table_get(struct table_t *table, char *key) {

  struct entry_t* this = get_entry(table,key);
  struct data_t* result = NULL;
     if(this!= NULL)
        result= data_dup(this->value);
  
    return result;
    
  }

/* Devolve o número de elementos na tabela.
*/
int table_size(struct table_t *table) {

  return table -> filled;

}

/* Devolve um array de char * com a cópia de todas as keys da
* tabela, e um último elemento a NULL.
*/
char **table_get_keys(struct table_t *table) {
  
  int i,j;
  j = 0;
  int size = table -> datasize;
  char** result = (char**) malloc(sizeof(char**)*(size + 1));

  for(i = 0; i < size; i++){
    if((table->entrys[i]).key!=NULL){
      result[j] = strdup((table->entrys[i]).key);
        j++;}
  }
  result[j] = NULL;

  return result;
}

/* Liberta a memória alocada por table_get_keys().
*/
void table_free_keys(char **keys) {

  int i = 0;
  while(keys[i]!= NULL){
    free(keys[i]);
    i++;
  }
  free(keys);
  }

void print_tables(struct table_t *table){
  int i;
  struct entry_t* currEntry;
  char *key;
  printf("-------------------\n");
  for(i = 0; i < table -> datasize; i++){
    printf("%d:",i);
    currEntry = &(table -> entrys[i]);
    if((key = currEntry -> key)!= NULL){
      printf(" %s ", key);
      currEntry = currEntry -> next;
      while(currEntry != NULL){
        printf(" %s",currEntry -> key);
        currEntry = currEntry -> next;
      }
    } 
    printf("\n"); 
  }
  printf("-------------------\n");
}
            