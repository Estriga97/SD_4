//Nomes: António Estriga, Diogo Caria Ferreira, Francisco Caeiro
//Numeros: 47839, 47840, 47823
//Grupo: 40
/*
	Programa cliente para manipular tabela de hash remota.
	Os comandos introduzido no programa não deverão exceder
	80 carateres.

	Uso: table-client <ip servidor>:<porta servidor>
	Exemplo de uso: ./table_client 10.101.148.144:54321
*/

#include <signal.h>

#include "network_client-private.h"
#include "client_stub-private.h"

#define MAX_READ 81
#define MAX_TENTATIVA 2

int main(int argc, char **argv){

	/* Testar os argumentos de entrada */
	if(argc != 3){
		fprintf(stderr, "Argumentos de entrada errados! ./client IP:port IP:port \n");
		return -1;
	}

	    
	char input [MAX_READ];
	signal(SIGPIPE, SIG_IGN);
    struct rtables_t* rtables = NULL;//POR A NULL EM TODOS OS ERROS

	/* Fazer ciclo até que o utilizador resolva fazer "quit" */
	int condicao = -1;
 	while (condicao){

		printf(">>> "); // Mostrar a prompt para inserção de comando

		if(rtables == NULL){

			int tentativas = 0;
			//tentar as conecçoes ao servidor principal seguido do servidor secundario
			do {
				if(tentativas > 0){
					fprintf(stderr, "Tentar ligaçao outra vez! \n");
					sleep(RETRY_TIME);
				}
					/* Usar network_connect para estabelcer ligação ao servidor principal */
					if((rtables = rtables_bind(argv[1])) == NULL){
						fprintf(stderr, "Erro a estabelecer ligaçao ao servidor principal! \n");
						rtables=NULL;
					}		
					else{
						rtables -> server -> server_type = 1;
						fprintf(stderr, "Ligado ao servidor principal. \n");
					
					}
					if(rtables==NULL){
					/* Usar network_connect para estabelcer ligação ao servidor secundario */
					if((rtables = rtables_bind(argv[2])) == NULL){
						fprintf(stderr, "Erro a estabelecer ligaçao ao servidor secundario! \n");
						rtables=NULL;
					}
					else{
						rtables -> server -> server_type = 0;
						fprintf(stderr, "Ligado ao servidor secundario. \n");
					}
					tentativas++;}
			}
			while(tentativas < MAX_TENTATIVA && rtables == NULL );

			if(rtables == NULL) {
				fprintf(stderr, "Nenhum servidor online \n");
				return -1;
			}

			rtables -> server -> ip_port_primario = strdup(argv[1]);
			rtables -> server -> ip_port_secundario = strdup(argv[2]);
		}
		
		/* Receber o comando introduzido pelo utilizador
		   Sugestão: usar fgets de stdio.h
		   Quando pressionamos enter para finalizar a entrada no
		   comando fgets, o carater \n é incluido antes do \0.
		   Convém retirar o \n substituindo-o por \0.
		*/
		fgets(input, MAX_READ, stdin);
		input[strlen(input)-1] = '\0';

		char spliters[] = " ";
		char* comando = strtok(input, spliters);

		/* Verificar se o comando foi "quit". Em caso afirmativo
		   não há mais nada a fazer a não ser terminar decentemente.
		 */
		if(!strcasecmp(comando,"quit")){
			condicao = 0;
			rtables_unbind(rtables);
		}

		/* Caso contrário:

			Verificar qual o comando;

			Preparar msg_out;

			Usar network_send_receive para enviar msg_out para
			o server e receber msg_resposta.
		*/
		else{
			if(!strcasecmp(comando,"put")){
			
				rtables->table_num = atoi(strtok(NULL, spliters));

				char* key = strdup(strtok(NULL, spliters));
                struct data_t* value;
				if((value = (struct data_t*) malloc(sizeof(struct data_t))) == NULL) {
					fprintf(stderr, "Erro ao alocar memoria no put! \n");
					free(key);
					free(rtables);
					return -1;
				}
				char* data= strtok(NULL, spliters);
				if((value -> data = malloc(strlen(data))) == NULL) {
					fprintf(stderr, "Erro ao alocar memoria no put! \n");
					free(key);
					free(value);
					free(rtables);
					return -1;
				}
				memcpy(value -> data, data, strlen(data));
				value -> datasize = strlen(data);

				if(rtables_put(rtables, key,value) == -1){
					fprintf(stderr, "Erro no commando put! \n");
					rtables_unbind(rtables);
				}
				else{
					free(key);
					free(value-> data);
					free(value);
				}
			}

			else if(!strcasecmp(comando, "get")){
				char* key;
				char* key_aux;
				rtables -> table_num = atoi(strtok(NULL, spliters));

				if(!strcmp((key_aux = strtok(NULL, spliters)),"*")){
					char** keys = rtables_get_keys(rtables);
					if(keys == NULL){
						fprintf(stderr, "Erro no commando get! \n");
						rtables_unbind(rtables);
					}
					else{
						rtables_free_keys(keys);
					}
				}

				else{
					key = strdup(key_aux);
					struct data_t* datatemp = rtables_get(rtables, key);
					if(datatemp == NULL){
						fprintf(stderr, "Erro no commando get! \n");
						rtables_unbind(rtables);
					}
					else{
						free(datatemp -> data);
						free(datatemp);
						free(key);
					}
				}
			}

			else if(!strcasecmp(comando, "update")){
				rtables->table_num = atoi(strtok(NULL, spliters));

				char* key = strdup(strtok(NULL, spliters));
              
                struct data_t* value;
				if((value = (struct data_t*) malloc(sizeof(struct data_t))) == NULL) {
					fprintf(stderr, "Erro ao alocar memoria no update! \n");
					free(key);
					free(rtables);
					return -1;
				}
				char* data= strtok(NULL, spliters);
				if((value -> data = malloc(strlen(data))) == NULL) {
					fprintf(stderr, "Erro ao alocar memoria no update! \n");
					free(key);
					free(value);
					free(rtables);
					return -1;
				}
				memcpy(value -> data, data, strlen(data));
				value -> datasize = strlen(data);

				if(rtables_update(rtables, key,value) == -1){
					fprintf(stderr, "Erro no commando uptade! \n");
					rtables_unbind(rtables);
				}
				else{
					free(key);
					free(value -> data);
					free(value);
				}
			}

			else if(!strcasecmp(comando, "size")){
				rtables->table_num = atoi(strtok(NULL, spliters));

				if(rtables_size(rtables) == -1){
					fprintf(stderr, "Erro no commando size! \n");
					rtables_unbind(rtables);
				}
				
			}

			else if(!strcasecmp(comando, "collisions")){
				rtables->table_num = atoi(strtok(NULL, spliters));
				if(rtables_collisions(rtables)==-1){
					fprintf(stderr, "Erro no commando collisions! \n");
					rtables_unbind(rtables);
				}
			}
			
			else{
				printf("Comando errado! \n");				

			}		
		}
	}
	return 0;
}

