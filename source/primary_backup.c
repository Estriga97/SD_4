#include "inet.h"
#include "message-private.h"
#include "table_server.h"
#include "primary_backup-private.h"
#include "network_server.h"


/* Função usada para um servidor avisar o servidor “server” de que
* já acordou. Retorna 0 em caso de sucesso, -1 em caso de insucesso
*/
int hello(struct server_t *server) {
    int res;
    if((res = server_connect(server)) < 0 ) {
        sleep(5);
        res = server_connect(server);
        if(res < 0)
            return -1;
    }
    int result;
    read_all(server->socket,(char*)&result, _INT);//Lixo
    rtables_hello(server);
    return 0;
}

/* Pede atualizacao de estado ao server.
* Retorna 0 em caso de sucesso e -1 em caso de insucesso.
*/
int update_state(struct server_t *server) {
    int res;
    if(hello(server) == -1 ){
        
    }
    int* ack;
        if((ack = (int*) malloc(sizeof(int))) == NULL) {
            //*
        }
    *ack = 0;

    while(!*ack) {
        if((res = network_receive_send(server->socket, ack)) < 0)
            return -1;
    }
        
    free(ack);

    return res;

}
int rtables_ip_port(struct server_t *server,char* ip_port, int size) {
    struct message_t* msg_tables;
    if((msg_tables = (struct message_t*) malloc(sizeof(struct message_t))) == NULL) {
        fprintf(stderr, "Erro ao alocar memoria \n");
        return -1;
    }
    
    msg_tables -> opcode = OC_IP_PORT;
    msg_tables -> c_type = CT_KEY;
    msg_tables -> table_num = -1;

    char* cnt_key = strdup(ip_port);//TODO:

    msg_tables->content.key = cnt_key;
    struct message_t* msg_resposta = network_send_receive(server, msg_tables);
    if(msg_resposta == NULL){
        imprimir_resposta(messgerror ());
        free(msg_resposta);
        free_message(msg_tables);
        return -1;
    }
    imprimir_resposta(msg_resposta);
    free_message(msg_tables);
    free(msg_resposta);
    return 0;
}
//////////////////////////////////// rtables_sz_tbles ////////////////////////////////////////////////////////

int rtables_sz_tbles(struct server_t *server,char** lst_tbls, int size) {
    struct message_t* msg_tables;
    if((msg_tables = (struct message_t*) malloc(sizeof(struct message_t))) == NULL) {
        fprintf(stderr, "Erro ao alocar memoria \n");
        return -1;
    }
    
    msg_tables -> opcode = OC_SZ_TABLES;
    msg_tables -> c_type = CT_KEYS;
    msg_tables -> table_num = -1;

    char** cnt_keys;
    if((cnt_keys = (char**) malloc(sizeof(char*)*(size+1))) == NULL)
        return -1;
        
    int i;
    for(i = 0; i < size; i++) {
        if((cnt_keys[i] = strdup(lst_tbls[i])) == NULL) {
            while(i > 0) {
                free(cnt_keys[i]);
                i--;
            }
            free(cnt_keys);
            return -1;
        }
    }
    cnt_keys[size] = NULL;

    msg_tables->content.keys = cnt_keys;
    struct message_t* msg_resposta = network_send_receive(server, msg_tables);
    if(msg_resposta == NULL){
        imprimir_resposta(messgerror ());
        free(msg_resposta);
        free_message(msg_tables);
        return -1;
    }
    imprimir_resposta(msg_resposta);
    free_message(msg_tables);
    free(msg_resposta);
    return 0;
}

//////////////////////////////////// rtables_ack ////////////////////////////////////////////////////////

int rtables_ack(struct server_t *server) {
    struct message_t* msg_ack;
    if((msg_ack = (struct message_t*) malloc(sizeof(struct message_t))) == NULL) {
        fprintf(stderr, "Erro ao alocar memoria \n");
        return -1;
    }
        
    msg_ack -> opcode = OC_ACK;
    msg_ack -> c_type = CT_RESULT;
    msg_ack -> table_num = -1;
    msg_ack -> content.result = 0;/////// VE ISTO ESTUSPIDO97

        
    struct message_t* msg_resposta = network_send_receive(server, msg_ack);
    if(msg_resposta == NULL){
        imprimir_resposta(messgerror ());
        free(msg_resposta);
        free_message(msg_ack);
        return -1;
    }
    imprimir_resposta(msg_resposta);
    free(msg_resposta);
    free(msg_ack);
    return 0;
}



/////////////////////////////////// rtables_put /////////////////////////////////////////////////////////


int rtables_put(struct server_t *server,int table_num, char *key, struct data_t *value){

    if(server == NULL || key == NULL || value == NULL){
        fprintf(stderr, "Argumentos NULL \n");
		return -1;
    }
    
    struct message_t* msg_out;

    if((msg_out = (struct message_t*) malloc(sizeof(struct message_t))) == NULL) {
        fprintf(stderr, "Erro ao alocar memoria \n");
        return -1;
    }

    msg_out -> opcode = OC_PUT;
    msg_out -> c_type = CT_ENTRY;
    msg_out -> table_num = table_num;
    if((msg_out -> content.entry = (struct entry_t*) malloc(sizeof(struct entry_t))) == NULL) {
        free(msg_out);
        return -1;
    }
    msg_out -> content.entry -> key = strdup(key);
    msg_out -> content.entry -> value = data_dup(value);
    imprimir_resposta(msg_out);
    struct message_t* msg_resposta;
    if((msg_resposta = network_send_receive(server, msg_out)) == NULL) {
        fprintf(stderr, "Erro ao enviar/receber mensagem \n");
        return -1;
    }
    
    imprimir_resposta(msg_resposta);
    free_message(msg_resposta);
    free_message(msg_out);
    return 0;
}

/////////////////////////////////// rtables_update /////////////////////////////////////////////////////////

int rtables_update(struct server_t *server,int table_num, char *key, struct data_t *value){
    struct message_t* msg_out;

    if(server == NULL || key == NULL || value == NULL){
        fprintf(stderr, "Argumentos NULL \n");
		return -1;
    }

    if((msg_out = (struct message_t*) malloc(sizeof(struct message_t))) == NULL) {
        fprintf(stderr, "Erro ao alocar memoria \n");
        return -1;
    }

    msg_out -> opcode = OC_UPDATE;
    msg_out -> c_type = CT_ENTRY;
    msg_out -> table_num = table_num;
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
    if((msg_resposta = network_send_receive(server, msg_out)) == NULL) {
        fprintf(stderr, "Erro ao enviar/receber mensagem \n");
        return -1;
        
    }
    imprimir_resposta(msg_resposta);
    free_message(msg_resposta);
    free_message(msg_out);

    return 0;

}

int rtables_hello(struct server_t *server) {
    struct message_t* msg_ack;
    if((msg_ack = (struct message_t*) malloc(sizeof(struct message_t))) == NULL) {
        fprintf(stderr, "Erro ao alocar memoria \n");
        return -1;
    }
        
    msg_ack -> opcode = OC_HELLO;
    msg_ack -> c_type = CT_RESULT;
    msg_ack -> table_num = -1;
    msg_ack -> content.result = 0;

        
    struct message_t* msg_resposta = network_send_receive(server, msg_ack);
    if(msg_resposta == NULL){
        imprimir_resposta(messgerror ());
        free(msg_resposta);
        free_message(msg_ack);
        return -1;
    }
    imprimir_resposta(msg_resposta);
    free(msg_resposta);
    free(msg_ack);
    return 0;
}