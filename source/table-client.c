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

	struct rtables_t* rtables;
	    
	char input [MAX_READ];
	signal(SIGPIPE, SIG_IGN);
	int tentativas = 0;

	//tentar as conecçoes ao servidor principal seguido do servidor secundario
   do {
	   if(tentativas > 0){
		   fprintf(stderr, "Tentar ligaçao outra vez!");
		   sleep(RETRY_TIME);
	   }
		/* Usar network_connect para estabelcer ligação ao servidor principal */
		if((rtables = rtables_bind(argv[1])) == NULL){
			fprintf(stderr, "Erro a establecer ligaçao ao servidor principal!");
		}		
		else{
			rtables -> server -> server_type = 0;
			fprintf(stderr, "Ligado ao servidor principal.");
		}

		/* Usar network_connect para estabelcer ligação ao servidor secundario */
		if((rtables == NULL && rtables = rtables_bind(argv[2])) == NULL){
			fprintf(stderr, "Erro a establecer ligaçao ao servidor secundario!");
		}
		else{
			rtables -> server -> server_type = 1;
			fprintf(stderr, "Ligado ao servidor secundario.");
		}
		tentativas++;
   }
   while(rtables == NULL && tentativas < MAX_TENTATIVA);


	/* Fazer ciclo até que o utilizador resolva fazer "quit" */
	int condicao = -1;
 	while (condicao){

		printf(">>> "); // Mostrar a prompt para inserção de comando
		
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
			free(rtables -> server);
			free(rtables);
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
					fprintf(stderr, "Erro ao alocar memoria no put!");
					free(key);
					free(rtables);
					return -1;
				}
				char* data= strtok(NULL, spliters);
				if((value -> data = malloc(strlen(data))) == NULL) {
					fprintf(stderr, "Erro ao alocar memoria no put!");
					free(key);
					free(value);
					free(rtables);
					return -1;
				}
				memcpy(value -> data, data, strlen(data));
				value -> datasize = strlen(data);

				rtables_put(rtables, key,value);
				free(key);
				free(value-> data);
				free(value);
			}

			else if(!strcasecmp(comando, "get")){
				char* key;
				char* key_aux;
				rtables -> table_num = atoi(strtok(NULL, spliters));

				if(!strcmp((key_aux = strtok(NULL, spliters)),"*")){
					char** keys = rtables_get_keys(rtables);
					rtables_free_keys(keys);
				}

				else{
					key = strdup(key_aux);
					struct data_t* datatemp = rtables_get(rtables, key);
					free(datatemp -> data);
					free(datatemp);
					free(key);
				}
			}

			else if(!strcasecmp(comando, "update")){
				rtables->table_num = atoi(strtok(NULL, spliters));

				char* key = strdup(strtok(NULL, spliters));
              
                struct data_t* value;
				if((value = (struct data_t*) malloc(sizeof(struct data_t))) == NULL) {
					fprintf(stderr, "Erro ao alocar memoria no update!");
					free(key);
					free(rtables);
					return -1;
				}
				char* data= strtok(NULL, spliters);
				if((value -> data = malloc(strlen(data))) == NULL) {
					fprintf(stderr, "Erro ao alocar memoria no update!");
					free(key);
					free(value);
					free(rtables);
					return -1;
				}
				memcpy(value -> data, data, strlen(data));
				value -> datasize = strlen(data);

				rtables_update(rtables, key,value);
				free(key);
				free(value -> data);
				free(value);
			}

			else if(!strcasecmp(comando, "size")){
				rtables->table_num = atoi(strtok(NULL, spliters));
				rtables_size(rtables);
			}

			else if(!strcasecmp(comando, "collisions")){
				rtables->table_num = atoi(strtok(NULL, spliters));
				rtables_collisions(rtables);
			}
			else{
				printf("Comando errado!\n");				

			}		
		}
	}
	return 0;
}

