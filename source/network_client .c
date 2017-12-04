//Nomes: António Estriga, Diogo Ferreira, Francisco Caeiro
//Numeros: 47839, 47840, 47823
//Grupo: 40

#include <stdlib.h>

#include "network_client-private.h"
#include "message-private.h"
#include "inet.h"

#define SLEEP_TIME 5
#define MAX_TENTATIVA 4

struct server_t *network_connect(const char *address_port){

	/* Verificar parâmetro da função e alocação de memória */
	if(address_port == NULL){
		return NULL;
	}

	struct server_t *server;
	if((server = malloc(sizeof(struct server_t))) == NULL) {
		fprintf(stderr, "Erro ao alocar memoria!");
		return NULL;
	}
	int sockfd;


	/* Estabelecer ligação ao servidor:

		Preencher estrutura struct sockaddr_in com dados do
		endereço do servidor.

		Criar a socket.

		Estabelecer ligação.
	*/
	struct sockaddr_in* addr;
	if((addr = malloc(sizeof(struct sockaddr_in))) == NULL) {
		fprintf(stderr, "Erro ao alocar memoria!");
		return NULL;
	}
	char* token = strtok((char*)address_port,":");
	printf("%s",token);
	fflush(stdout);

	addr-> sin_family = AF_INET;
	if (inet_pton(AF_INET, token, &(addr-> sin_addr)) < 1) {
		printf("Erro ao converter IP\n");
		return NULL;
		}

	addr-> sin_port = htons(atoi(strtok(NULL,":")));

	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		fprintf(stderr, "Erro ao criar socket TCP!");
		return NULL;
	}

	// Estabelece conexão com o servidor definido em server
	if (connect(sockfd,(struct sockaddr *)addr, sizeof(*addr)) < 0) {
		fprintf(stderr, "Erro ao conectar-se ao servidor!");
		close(sockfd);
		return NULL;
}
	/* Se a ligação não foi estabelecida, retornar NULL */
	server -> socket = sockfd;

	free(addr);
	return server;
}

struct message_t *network_send_receive(struct server_t *server, struct message_t *msg){

	/* Verificar parâmetros de entrada */
	if(server == NULL || msg == NULL) {
		fprintf(stderr, "Erro no contudo recebido (server ou msg)!");
		return messgerror ();
	}

	char *message_out;
	int message_size, msg_size, result;
	int tentativas = 0;
	struct message_t* msg_resposta;



	/* Serializar a mensagem recebida */
	message_size = message_to_buffer(msg, &message_out);
   
	/* Verificar se a serialização teve sucesso */
	if(message_size == -1) {
		fprintf(stderr, "Erro no tamanho da mensagem!");
		free(message_out);
		free_message(msg);
		return messgerror ();}

	/* Enviar ao servidor o tamanho da mensagem que será enviada
	   logo de seguida
	*/
	msg_size = htonl(message_size);


	do{
		if(result = write_all(server->socket, (char *) &msg_size, _INT)) == -1){
			sleep(SLEEP_TIME);
			if(server -> server_type){
				network_connect(server -> ip_port_secundario);
				server -> server_type = 0;
			}
			else{
				network_connect(server -> ip_port_primario);
				server -> server_type = 1;
			}
		}	

		tentativas++;
	}
	while(result == -1 && tentativas < MAX_TENTATIVA);

	/* Verificar se o envio teve sucesso */
	if(result == -1) {
		fprintf(stderr, "Erro ao enviar!");
		free(message_out);
		free_message(msg);
		return messgerror ();
	}

	//reset nas tentativas de conecçao
	tentativas = 0;

	/* Enviar a mensagem que foi previamente serializada */
	do{	
		if(result = write_all(server->socket, message_out, message_size)) == -1){
			sleep(SLEEP_TIME);
			if(server -> server_type){
				network_connect(server -> ip_port_secundario);
				server -> server_type = 0;
			}
			else{
				network_connect(server -> ip_port_primario);
				server -> server_type = 1;
			}
		}
		
		tentativas++;

	} while(result == -1 && tentativas < MAX_TENTATIVA);

	if(result == -1) {
		fprintf(stderr, "Erro ao enviar!");
		free(message_out);
		free_message(msg);
		return messgerror ();
	}

	//reset nas tentativas de conecçao
	tentativas = 0;

	/* De seguida vamos receber a resposta do servidor:

		Com a função read_all, receber num inteiro o tamanho da 
		mensagem de resposta.

		Alocar memória para receber o número de bytes da
		mensagem de resposta.

		Com a função read_all, receber a mensagem de resposta.
		
	*/
	int size;
	i=0;
	while((result = read_all(server->socket, buff, size) ) == -1 && i!= 1){
		sleep(SLEEP_TIME);
		i++;
	 }

	if(result == -1) {
		fprintf(stderr, "Erro ao receber o tamanho da mensagem de resposta!");
		free(message_out);
		free_message(msg);
		return messgerror ();
	}

	//reset nas tentativas de conecçao
	tentativas = 0;

	size = ntohl(size);
	char* buff;
	if((buff = (char*) malloc(size)) == NULL) {
		fprintf(stderr, "Erro ao alocar memoria para a mensagem de resposta!");
		free(message_out);
		free_message(msg);
		return messgerror ();
	}

	while((result = read_all(server->socket, buff, size) ) == -1 && i!= 1){
		sleep(SLEEP_TIME);
		i++;
	 }
	 i = 0;

	if(result == -1) {
		fprintf(stderr, "Erro receber a mensagem de resposta!");
		free(message_out);
		free(msg);
		free(buff);
		return messgerror ();
	}

	/* Desserializar a mensagem de resposta */
	msg_resposta = buffer_to_message(buff, size);

	/* Verificar se a desserialização teve sucesso */
	if(msg_resposta == NULL) {
		fprintf(stderr, "Erro na desserialização!");
		free(message_out);
		free_message(msg_resposta);
		free(msg);
		free(buff);
		return messgerror ();
	}

	/* Libertar memória */
	free(message_out);
	free(buff);

	return msg_resposta;
}

int network_close(struct server_t *server){
	/* Verificar parâmetros de entrada */
	if(server == NULL) {
		fprintf(stderr, "Erro no close!");
		return -1;
	}

	/* Terminar ligação ao servidor */
	close(server -> socket);

	return 0;
}

