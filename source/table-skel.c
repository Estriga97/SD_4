
#include <error.h>
#include <stdlib.h>

#include "message-private.h"
#include "inet.h"
#include "table-private.h"
#include "table_skel.h"


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
    fprintf(stderr, "Argumentos errados!");
    return -1;
  }

  int size = 0;
  int i = 0;
  while(n_tables[size]!= NULL){
    size++;
  }
  n_tabelas = size;
  if((tabelas = (struct table_t**) malloc (sizeof(struct table_t**) * size)) == NULL){
    fprintf(stderr, "Erro ao preparar tabelas");
    return -1;
  }
  while(i < size) {
    tabelas[i] = table_create(atoi(n_tables[i]));
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
        fprintf(stderr, "Argumentos errados!");
        return NULL;
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
            fprintf(stderr, "Erro ao pedir put");
            return messgerror ();
          }
          else {  
            validade = table_put(tabela, msg_in -> content.entry -> key, msg_in -> content.entry -> value);
            if (validade != 0){
              fprintf(stderr, "Erro ao executar operação pedida (put)");
              return messgerror ();
            }
          }
        break;
    
        case OC_GET:
    
          if(ctp != CT_KEY){
            fprintf(stderr, "Erro ao pedir get");
            return messgerror ();
          }
          else {
            if(strcmp(msg_in -> content.key,"*"))
              val_get = table_get(tabela, msg_in -> content.key);
          }
            
        break;
    
        case OC_UPDATE:
    
        if(ctp != CT_ENTRY){
          fprintf(stderr, "Erro ao pedir update");
          return messgerror ();
        }
        else {
          validade = table_update(tabela, msg_in -> content.entry -> key, msg_in -> content.entry -> value);
          if (validade != 0){
            fprintf(stderr, "Erro ao executar operação pedida (update)");
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
        fprintf(stderr, "Erro ao alocar memoria para a mensagem de resposta");
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
            msg_resposta -> content.data = data_create2(0, NULL);}
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
        fprintf(stderr, "Tamanho do argumento errado!");
      }
    }
    

struct message_t invoke_server_version(struct message_t){

      table_skel_init(msg_pedido.content->keys);
      msg_resposta = malloc(sizeof(struct message_t));
      msg_resposta  -> opcode = OC_TABLES;
      msg_resposta  -> c_type = CT_SZ_TABLES;
      msg_resposta  -> table_num = -1; }


struct entry_t* get_tbl_keys(int n){
    struct table_t* tbl = tabelas[n];
    struct entry* tbl_res;
    if(tbl -> filled != 0) {
        return NULL;
    }
    else {
        tbl_res = (struct entry*) malloc(sizeof(struct entry));
        int i;
        for(i = 0; i < tbl -> filled; i++) {
            if((tbl_res[i] = entry_dup(tbl[i])) == NULL) {
                while(i > 0) {
                    free(tbl_res[i]);
                    i--;
                }
                free(tbl_res);
                return -1;    
            }
        }
        return tbl_res;
}