
#include <error.h>
#include <stdlib.h>

#include "message-private.h"
#include "inet.h"
#include "table-private.h"
#include "table_skel.h"
#include "primary_backup-private.h"

extern struct server_t *o_server;
struct table_t** tabelas; 
int n_tabelas;

/* Inicia o skeleton da tabela.
 * O main() do servidor deve chamar esta função antes de poder usar a
 * função invoke(). O parâmetro n_tables define o número e dimensão das
 * tabelas a serem mantidas no servidor.
 * Retorna 0 (OK) ou -1 (erro, por exemplo OUT OF MEMORY)
 */
int table_skel_init(char **n_tables){

  if(n_tables == NULL){
    fprintf(stderr, "Argumentos errados! \n");
    return -1;
  }

  int size = 0;
  int i = 0;
  while(n_tables[size]!= NULL){
    size++;
  }
  n_tabelas = size;
  if((tabelas = (struct table_t**) malloc (sizeof(struct table_t**) * size)) == NULL){
    fprintf(stderr, "Erro ao preparar tabelas \n");
    return -1;
  }
  while(i < size) {
    if((tabelas[i] = table_create(atoi(n_tables[i]))) == NULL) {
      while (i > 0) {
        free(tabelas[i]);
        i--;
      }
      return -1;
    }
    i++;
  }

  return 0;
}

/* Libertar toda a memória e recursos alocados pela função anterior.
 */
int table_skel_destroy(){
    int i = 0;
    while(i  < n_tabelas){
        table_destroy(tabelas[i++]);
      }
      free(tabelas);
      return 0;
    }

/* Executa uma operação numa tabela (indicada pelo opcode e table_num
 * na msg_in) e retorna o resultado numa mensagem de resposta ou NULL
 * em caso de erro.
 */
struct message_t *invoke(struct message_t *msg_in){

      /* Verificar parâmetros de entrada */
      if(msg_in == NULL){
        fprintf(stderr, "Argumentos errados! \n");
        return NULL;
      }

      if(msg_in -> table_num >= n_tabelas){
        fprintf(stderr, "Erro devido a inexistência de Tabela! \n");
        return messgerror();
    }

    struct message_t *msg_resposta;
    struct table_t * tabela = tabelas [msg_in-> table_num];

        /* Verificar opcode e c_type na mensagem de pedido */
      short opcd = msg_in -> opcode;
      short ctp = msg_in -> c_type;
    
      /* Aplicar operação na tabela */
      int validade,size,colls;
      struct data_t* val_get;
    
      switch (opcd) {
    
        case OC_PUT:
    
          if(ctp != CT_ENTRY){
            fprintf(stderr, "Erro ao pedir put \n");
            return messgerror ();
          }
          else {  
            validade = table_put(tabela, msg_in -> content.entry -> key, msg_in -> content.entry -> value);
            if (validade != 0){
              fprintf(stderr, "Erro ao executar operação pedida (put) \n");
              return messgerror ();
            }
          }
        break;
    
        case OC_GET:
    
          if(ctp != CT_KEY){
            fprintf(stderr, "Erro ao pedir get \n");
            return messgerror ();
          }
          else {
            if(strcmp(msg_in -> content.key,"*"))
              val_get = table_get(tabela, msg_in -> content.key);
          }
            
        break;
    
        case OC_UPDATE:
    
        if(ctp != CT_ENTRY){
          fprintf(stderr, "Erro ao pedir update \n");
          return messgerror ();
        }
        else {
          validade = table_update(tabela, msg_in -> content.entry -> key, msg_in -> content.entry -> value);
          if (validade != 0){
            fprintf(stderr, "Erro ao executar operação pedida (update) \n");
            return messgerror ();
          }
        }
        break;
    
        case OC_SIZE:
            size = table_size(tabela);
        break;
    
        case OC_COLLS:
            colls = tabela -> collisions;
        break;
    
        default:
          return messgerror ();
      }
    
        /* Preparar mensagem de resposta */
    
      if((msg_resposta = (struct message_t*) malloc(sizeof(struct message_t))) == NULL) {
        fprintf(stderr, "Erro ao alocar memoria para a mensagem de resposta \n");
        return messgerror ();
      }
    
      msg_resposta -> opcode = opcd+1;
      msg_resposta -> table_num = msg_in -> table_num;
    
    
      switch (opcd) {
        
        case OC_PUT:
          msg_resposta -> c_type = CT_RESULT;
          msg_resposta -> content.result = validade;
        break;
          
        case OC_GET:
          if (!strcmp(msg_in -> content.key,"*")){
            msg_resposta -> c_type = CT_KEYS;
            msg_resposta -> content.keys = table_get_keys(tabela);
            
          }
          else if(val_get== NULL){
            msg_resposta -> c_type = CT_VALUE;
            msg_resposta -> content.data = data_create2(0, NULL); // q fzr neste caso? averiguar à mesma?
          }
          else{
            msg_resposta -> c_type = CT_VALUE;
            msg_resposta -> content.data = val_get;
          }
        break;
          
        case OC_UPDATE:
          msg_resposta -> c_type = CT_RESULT;
          msg_resposta -> content.result = validade;
        break;
          
        case OC_SIZE:
          msg_resposta -> c_type = CT_RESULT;
          msg_resposta -> content.result = size;
        break;
          
        case OC_COLLS:
          msg_resposta -> c_type = CT_RESULT;
          msg_resposta -> content.result = colls;
        break;
          
        default:
        return messgerror ();
        }
        
        return msg_resposta;


      }
void get_keys(int n){
    if(n >= 0 && n < n_tabelas){
        print_tables(tabelas[n]);
    }      
    else{
        fprintf(stderr, "Tamanho do argumento errado! \n");
    }
}   

struct message_t* invoke_server_version(struct message_t* msg_pedido){

    struct message_t* msg_resposta;
    if((msg_resposta = (struct message_t*) malloc(sizeof(struct message_t))) == NULL) {
        fprintf(stderr, "Erro ao alocar memoria! \n");
        return NULL;
    }
    
    short opcode = msg_pedido->opcode;
    switch (opcode) {
        case OC_SZ_TABLES:
            if(table_skel_init(msg_pedido -> content.keys) == -1) {
                fprintf(stderr, "Erro ao inicializar tabelas! \n");
                return NULL;
            }
            msg_resposta -> opcode = OC_SZ_TABLES + 1;
            msg_resposta -> c_type =  CT_RESULT;
            msg_resposta -> table_num = -1; 
            msg_resposta -> content.result = 0;
            break;
        case OC_ACK:
            msg_resposta -> opcode = OC_ACK + 1;
            msg_resposta -> c_type = CT_RESULT;
            msg_resposta -> table_num = -1;
            msg_resposta -> content.result = 0;
            break;
        case OC_IP_PORT:
            msg_resposta -> opcode = OC_IP_PORT + 1;
            msg_resposta -> c_type = CT_RESULT;
            msg_resposta -> table_num = -1;
            msg_resposta -> content.result = 0;
            o_server->ip_port = strdup(msg_pedido->content.key);
            break;
        case OC_HELLO:
            msg_resposta -> opcode = OC_HELLO + 1;
            msg_resposta -> c_type = CT_RESULT;
            msg_resposta -> table_num = -1;
            msg_resposta -> content.result = 0;
            o_server->state = 1;
            break;
    }
    return msg_resposta;
}

struct entry_t* get_tbl_keys(int n) {
    struct table_t* tbl = tabelas[n];
    struct entry_t* tbl_res;
    struct entry_t* entry_aux; 
    struct entry_t* table_init;
    if(tbl -> filled == 0) {
        fprintf(stderr, "Sem chaves \n");
        return NULL;
    }
    else {
        if((tbl_res = (struct entry_t*) malloc(sizeof(struct entry_t*)*(tbl -> filled+1))) == NULL) {
            fprintf(stderr, "Erro ao alocar memoria! \n");
            return NULL;
        }
        table_init = tbl_res;
        int i;
        for(i = tbl -> datasize-1; i >= 0; i--) {
            entry_aux = &(tbl -> entrys[i]);
        }

        for(i = tbl -> datasize-1; i >= 0; i--) {
            if((tbl -> entrys[i]).key!= NULL){
                if((entry_aux = (&(tbl -> entrys[i]))) == NULL) {
                    while(i < (tbl -> filled)) {
                        free(tbl_res);
                        tbl_res--;
                        i++;
                    }
                    free(table_init);
                    return NULL;
                }
                (*tbl_res).key = strdup(entry_aux ->key);
                (*tbl_res).value = data_dup(entry_aux->value);
                tbl_res++;
                }
        }
        (*tbl_res).key = NULL;
        return table_init;
    }

}
