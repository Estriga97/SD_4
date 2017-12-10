

#include "client_stub-private.h"

/* Função para estabelecer uma associação entre o cliente e um conjunto de
 * tabelas remotas num servidor.
 * Os alunos deverão implementar uma forma de descobrir quantas tabelas
 * existem no servidor.
 * address_port é uma string no formato <hostname>:<port>.
 * retorna NULL em caso de erro .
 */
struct rtables_t *rtables_bind(const char *address_port){

    if(address_port == NULL){
        fprintf(stderr, "Argumentos NULL \n");
		return NULL;
    }

    struct rtables_t * rtables;
    if((rtables = (struct rtables_t*) malloc(sizeof(struct rtables_t))) == NULL){
        return NULL;
    }
    struct server_t *server;
    if((server = network_connect(address_port)) == NULL) {
        free(rtables);
        return NULL;
    }
    rtables -> server = server;
    int result;
    if(read_all(server->socket,(char*)&result, _INT) == -1) {
		fprintf(stderr, "Erro ao read_all! \n");
        free(rtables);
        free(server);
		return NULL;
    }
    rtables -> n_tables = ntohl(result);
    return rtables;
}

/* Termina a associação entre o cliente e um conjunto de tabelas remotas, e
 * liberta toda a memória local. 
 * Retorna 0 se tudo correr bem e -1 em caso de erro.
 */
int rtables_unbind(struct rtables_t *rtables){


    if(rtables == NULL){
        fprintf(stderr, "Argumentos NULL \n");
		return -1;
    }

    if(network_close(rtables -> server)){
        fprintf(stderr, "Falha a fechar o servidor! \n");
        return -1;
    }

    free(rtables);
    
    return 0;
}

/* Função para adicionar um par chave valor numa tabela remota.
 * Devolve 0 (ok) ou -1 (problemas).
 */
int rtables_put(struct rtables_t *rtables, char *key, struct data_t *value){

    if(rtables == NULL || key == NULL || value == NULL){
        fprintf(stderr, "Argumentos NULL \n");
		return -1;
    }
    
    struct message_t* msg_out;
    if(rtables-> n_tables<=rtables -> table_num){
        imprimir_resposta(messgerror ());
        return -1;
    }

    if((msg_out = (struct message_t*) malloc(sizeof(struct message_t))) == NULL) {
        fprintf(stderr, "Erro ao alocar memoria \n");
        return -1;
    }

    msg_out -> opcode = OC_PUT;
    msg_out -> c_type = CT_ENTRY;
    msg_out -> table_num = rtables->table_num;
    if((msg_out -> content.entry = (struct entry_t*) malloc(sizeof(struct entry_t))) == NULL) {
        free(msg_out);
        return -1;
    }
    msg_out -> content.entry -> key = strdup(key);
    msg_out -> content.entry -> value = data_dup(value);
    imprimir_resposta(msg_out);
    struct message_t* msg_resposta;
    if((msg_resposta = network_send_receive(rtables-> server, msg_out)) == NULL) {
        fprintf(stderr, "Erro ao enviar/receber mensagem \n");
        return -1;
        
    }
    imprimir_resposta(msg_resposta);
    free_message(msg_resposta);
    free_message(msg_out);
    return 0;
}

/* Função para substituir na tabela remota, o valor associado à chave key.
 * Devolve 0 (OK) ou -1 em caso de erros.
 */
int rtables_update(struct rtables_t *rtables, char *key, struct data_t *value){
    struct message_t* msg_out;

    if(rtables == NULL || key == NULL || value == NULL){
        fprintf(stderr, "Argumentos NULL \n");
		return -1;
    }

    if(rtables-> n_tables<=rtables -> table_num){
        imprimir_resposta(messgerror ());
        return -1;
    }
    if((msg_out = (struct message_t*) malloc(sizeof(struct message_t))) == NULL) {
        fprintf(stderr, "Erro ao alocar memoria \n");
        return -1;
    }

    msg_out -> opcode = OC_UPDATE;
    msg_out -> c_type = CT_ENTRY;
    msg_out -> table_num = rtables->table_num;
    if((msg_out -> content.entry = (struct entry_t*) malloc(sizeof(struct entry_t))) == NULL) {
        free(msg_out);
        free(msg_out -> content.entry);
        fprintf(stderr, "Erro ao alocar memoria \n");
        return -1;
    }
    msg_out -> content.entry -> key = strdup(key);
    msg_out -> content.entry -> value = data_dup(value);
    imprimir_resposta(msg_out);

    struct message_t* msg_resposta;
    if((msg_resposta = network_send_receive(rtables-> server, msg_out)) == NULL) {
        fprintf(stderr, "Erro ao enviar/receber mensagem \n");
        return -1;
        
    }
    imprimir_resposta(msg_resposta);
    free_message(msg_resposta);
    free_message(msg_out);

    return 0;

}

/* Função para obter da tabela remota o valor associado à chave key.
 * Devolve NULL em caso de erro.
 */
struct data_t *rtables_get(struct rtables_t *rtables, char *key){
    struct message_t* msg_out;

    if(rtables == NULL || key == NULL){
        fprintf(stderr, "Argumentos NULL \n");
		return NULL;
    }

   if(rtables-> n_tables<=rtables -> table_num){
        imprimir_resposta(messgerror ());
        return NULL;
    }    

    if((msg_out = malloc(sizeof(struct message_t))) == NULL) {
        fprintf(stderr, "Erro ao alocar memoria \n");
        return NULL;
    }

    msg_out -> opcode = OC_GET;
    msg_out -> c_type = CT_KEY;
    msg_out -> table_num = rtables->table_num;
    msg_out -> content.key = strdup(key);
    imprimir_resposta(msg_out);

    struct message_t* msg_resposta;
    if((msg_resposta = network_send_receive(rtables-> server, msg_out)) == NULL) {
            fprintf(stderr, "Erro ao enviar/receber mensagem \n");
            return NULL;
    }
    struct data_t* res = data_dup(msg_resposta -> content.data);
    imprimir_resposta(msg_resposta);
    free_message(msg_resposta);
    free_message(msg_out);
    
    return res;

}

/* Devolve número de pares chave/valor na tabela remota.
 */
int rtables_size(struct rtables_t *rtables){
    struct message_t* msg_out;

    if(rtables == NULL){
        fprintf(stderr, "Argumentos NULL \n");
		return -1;
    }

    if(rtables-> n_tables<=rtables -> table_num){
        imprimir_resposta(messgerror ());
        return -1;
    }
    if((msg_out = (struct message_t*) malloc(sizeof(struct message_t))) == NULL) {
        fprintf(stderr, "Erro ao alocar memoria \n");
        return -1;
    }

    msg_out -> opcode = OC_SIZE;
    msg_out -> c_type = 0;
    msg_out -> table_num = rtables->table_num;
    imprimir_resposta(msg_out);

    struct message_t* msg_resposta;
    if((msg_resposta = network_send_receive(rtables-> server, msg_out)) == NULL) {
        fprintf(stderr, "Erro ao enviar/receber mensagem \n");
        return -1;
        
    }
    imprimir_resposta(msg_resposta);
    int size = msg_resposta -> content.result;
    free_message(msg_resposta);
    free_message(msg_out);

    return size;

}


int rtables_collisions(struct rtables_t *rtables){
    struct message_t* msg_out;

    if(rtables == NULL){
        fprintf(stderr, "Argumentos NULL \n");
		return -1;
    }
    if(rtables-> n_tables <= rtables -> table_num){
        imprimir_resposta(messgerror ());
        return -1;
    }
    if((msg_out = malloc(sizeof(struct message_t))) == NULL) {
        fprintf(stderr, "Erro ao alocar memoria \n");
        return -1;
    }

    msg_out -> opcode = OC_COLLS;
    msg_out -> c_type = 0;
    msg_out -> table_num = rtables->table_num;
    imprimir_resposta(msg_out);

    struct message_t* msg_resposta;
    if((msg_resposta = network_send_receive(rtables-> server, msg_out)) == NULL) {
        fprintf(stderr, "Erro ao enviar/receber mensagem \n");
        return -1;
        
    }
    imprimir_resposta(msg_resposta);
    int colls = msg_resposta -> content.result;
    free_message(msg_resposta);
    free_message(msg_out);

return colls;

}

/* Devolve um array de char * com a cópia de todas as keys da
 * tabela remota, e um último elemento a NULL.
 */
char **rtables_get_keys(struct rtables_t *rtables) {
    struct message_t* msg_out;

    if(rtables == NULL){
        fprintf(stderr, "Argumentos NULL \n");
		return NULL;
    }
    if(rtables-> n_tables<=rtables -> table_num){
        imprimir_resposta(messgerror ());
        return NULL;
    }
    if((msg_out = (struct message_t*) malloc(sizeof(struct message_t))) == NULL) {
        fprintf(stderr, "Erro ao alocar memoria \n");
        return NULL;
    }

    msg_out -> opcode = OC_GET;
    msg_out -> c_type = CT_KEY;
    msg_out -> table_num = rtables -> table_num;
    msg_out -> content.key = "*";
    imprimir_resposta(msg_out);

    struct message_t* msg_resposta;
    if((msg_resposta = network_send_receive(rtables-> server, msg_out)) == NULL) {
            fprintf(stderr, "Erro ao enviar/receber mensagem \n");
            return NULL;
    }
    imprimir_resposta(msg_resposta);
    int size = 0;

    while(msg_resposta ->content.keys[size]!= NULL){
        size++;
    }
    char ** keys;
    if((keys = (char**) malloc(sizeof(char**)*(size + 1))) == NULL) {
        fprintf(stderr, " Erro ao preparar keys \n");
        return NULL;
    }

    int i = 0;
    while(msg_resposta ->content.keys[i]!= NULL){
        keys[i] = strdup(msg_resposta -> content.keys[i]);
      i++;
    }  
    keys[size] = NULL;
    free_message(msg_resposta);
    free(msg_out);

    return keys;

}

/* Liberta a memória alocada por rtables_get_keys().
 */
void rtables_free_keys(char **keys){
    if(keys != NULL){
        table_free_keys(keys);
    }
    else{
        fprintf(stderr, "Erro ao libertar key! Keys NULL! \n");
    }
}


