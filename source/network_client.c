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
		fprintf(stderr, "Erro ao alocar memoria! \n");
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
		fprintf(stderr, "Erro ao alocar memoria! \n");
		free(server);
		return NULL;
	}
	char* bckup = strdup(address_port);
	char* token = strtok((char*) bckup,":");

	addr-> sin_family = AF_INET;
	if (inet_pton(AF_INET, token, &(addr-> sin_addr)) < 1) {
		printf("Erro ao converter IP\n");
		free(addr);
		free(server);
		free(bckup);
		return NULL;
		}

	addr-> sin_port = htons(atoi(strtok(NULL,":")));

	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		fprintf(stderr, "Erro ao criar socket TCP! \n");
		free(addr);
		free(server);
		free(bckup);
		return NULL;
	}

	// Estabelece conexão com o servidor definido em server
	if (connect(sockfd,(struct sockaddr *)addr, sizeof(*addr)) < 0) {
		fprintf(stderr, "Erro ao conectar-se ao servidor! \n");
		free(server);
		free(addr);
		free(bckup);
		close(sockfd);
		return NULL;
}
	/* Se a ligação não foi estabelecida, retornar NULL */
	server -> socket = sockfd;

	free(addr);
	free(bckup);

	return server;
}

struct message_t *network_send_receive(struct server_t *server, struct message_t *msg){
	int sucesso = 0;
	int tentativas = 0;
	/* Verificar parâmetros de entrada */
	if(server == NULL || msg == NULL) {
		fprintf(stderr, "Erro no contudo recebido (server ou msg)! \n");
		return messgerror ();
	}
	
	char *message_out;
	int message_size, msg_size, result,i;
	struct message_t* msg_resposta;

	do{

	/* Serializar a mensagem recebida */
	message_size = message_to_buffer(msg, &message_out);
   
	/* Verificar se a serialização teve sucesso */
	if(message_size == -1) {
		fprintf(stderr, "Erro no tamanho da mensagem! \n");
		tentativas++;
		if((switch_server(server)) == -1) {
			return NULL;
		}
		continue;
	}

	/* Enviar ao servidor o tamanho da mensagem que será enviada
	   logo de seguida
	*/
	msg_size = htonl(message_size);
 	while((result = write_all(server->socket, (char *) &msg_size, _INT)) == -1 && i!= 1){
		sleep(5);
		i++;
	}
	i = 0;
	/* Verificar se o envio teve sucesso */
	if(result == -1) {
		fprintf(stderr, "Erro ao enviar! \n");
		free(message_out);
		tentativas++;
		if((switch_server(server)) == -1) {
			return NULL;
		}
		continue;;
	}

	/* Enviar a mensagem que foi previamente serializada */
	while((result = write_all(server->socket, message_out, message_size)) == -1 && i!= 1){
		sleep(5);
		i++;
	}
	i = 0;
	/* Verificar se o envio teve sucesso */
	if(result == -1) {
		fprintf(stderr, "Erro ao enviar! \n");
		free(message_out);
		tentativas++;
		if((switch_server(server)) == -1) {
			return NULL;
		}
		continue;
	}

	/* De seguida vamos receber a resposta do servidor:

		Com a função read_all, receber num inteiro o tamanho da 
		mensagem de resposta.

		Alocar memória para receber o número de bytes da
		mensagem de resposta.

		Com a função read_all, receber a mensagem de resposta.
		
	*/
	int size;
	while((result = read_all(server->socket,(char*) &size, _INT) ) == -1 && i!= 1){
		sleep(5);
		i++;
	}
	i = 0;
	
	if(result == -1) {
		fprintf(stderr, "Erro ao receber o tamanho da mensagem de resposta! \n");
		free(message_out);
		tentativas++;
		if((switch_server(server)) == -1) {
			return NULL;
		}
		continue;
	}

	size = ntohl(size);
	char* buff;
	if((buff = (char*) malloc(size)) == NULL) {
		fprintf(stderr, "Erro ao alocar memoria para a mensagem de resposta! \n");
		free(message_out);
		tentativas++;
		if((switch_server(server)) == -1) {
			return NULL;
		}
		continue;
	}

	while((result = read_all(server->socket, buff, size) ) == -1 && i!= 1){
		sleep(5);
		i++;
	 }
	 i = 0;

	if(result == -1) {
		fprintf(stderr, "Erro receber a mensagem de resposta! \n");
		free(message_out);
		free(buff);
		tentativas++;
		if((switch_server(server)) == -1) {
			return NULL;
		}
		continue;
	}

	/* Desserializar a mensagem de resposta */
	msg_resposta = buffer_to_message(buff, size);

	/* Verificar se a desserialização teve sucesso */
	if(msg_resposta == NULL) {
		fprintf(stderr, "Erro na desserialização! \n");
		free(message_out);
		free_message(msg_resposta);
		free(buff);
		tentativas++;
		if((switch_server(server)) == -1) {
			return NULL;
		}
		continue;
	}

	/* Libertar memória */
	free(message_out);
	free(buff);
	sucesso = 1;
}
	while(!sucesso && tentativas < MAX_TENTATIVA);

	if(!sucesso){
		msg_resposta = messgerror ();
	}
	return msg_resposta;
}

int network_close(struct server_t *server){
	/* Verificar parâmetros de entrada */
	if(server == NULL) {
		fprintf(stderr, "Erro no close! \n");
		return -1;
	}

	/* Terminar ligação ao servidor */
	close(server -> socket);
	free(server -> ip_port_primario);
	free(server -> ip_port_secundario);

	return 0;
}


int switch_server(struct server_t *server){
	struct server_t *new_server;

	if((new_server = network_connect((server)->ip_port_secundario)) == NULL) {
			fprintf(stderr, "Erro ao conectar ao outro servidor \n");
			return -1;
		}
	new_server -> ip_port_secundario = strdup(server -> ip_port_primario);
	new_server -> ip_port_primario =  strdup(server -> ip_port_secundario);
	
	network_close(server);
	server -> ip_port_primario = new_server -> ip_port_primario;
	server -> ip_port_secundario = new_server -> ip_port_secundario;
	server -> socket = new_server -> socket;

	free(new_server);
	return 0;
	
}
