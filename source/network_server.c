
#include "network_server.h"
#include "message-private.h"


//////////////////////////////////// network_send_receive ////////////////////////////////////////////////////////

struct message_t *network_send_receive(struct server_t *server, struct message_t *msg){

	/* Verificar parâmetros de entrada */
	if(server == NULL || msg == NULL) {
		fprintf(stderr, "Erro no contudo recebido (server ou msg)! \n");
		return messgerror ();
	}

	char *message_out;
	int message_size, msg_size, result,i;
	struct message_t* msg_resposta;

	/* Serializar a mensagem recebida */
	message_size = message_to_buffer(msg, &message_out);
   
	/* Verificar se a serialização teve sucesso */
	if(message_size == -1) {
		fprintf(stderr, "Erro no tamanho da mensagem! \n");
		free(message_out);
		free_message(msg);
		return messgerror ();
	}

	/* Enviar ao servidor o tamanho da mensagem que será enviada logo de seguida */
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
		free_message(msg);
		return messgerror ();
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
		free_message(msg);
		return messgerror ();
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
		//sleep(5); ?????? TODO!!
		i++;
	 }
	 i = 0;
	
	if(result == -1) {
		fprintf(stderr, "Erro ao receber o tamanho da mensagem de resposta! \n");
		free(message_out);
		free_message(msg);
		return messgerror ();
	}
	printf("%d",size);
	size = ntohl(size);
	char* buff;
	printf("%d",size);
	fflush(stdout);
	if((buff = (char*) malloc(size)) == NULL) {
		fprintf(stderr, "Erro ao alocar memoria para a mensagem de resposta! \n");
		free(message_out);
		free_message(msg);
		return messgerror ();
	}

	while((result = read_all(server->socket, buff, size) ) == -1 && i!= 1){
		sleep(5);
		i++;
	 }
	 i = 0;

	if(result == -1) {
		fprintf(stderr, "Erro receber a mensagem de resposta! \n");
		free(message_out);
		free(msg);
		free(buff);
		return messgerror ();
	}

	/* Desserializar a mensagem de resposta */
	msg_resposta = buffer_to_message(buff, size);

	/* Verificar se a desserialização teve sucesso */
	if(msg_resposta == NULL) {
		fprintf(stderr, "Erro na desserialização! \n");
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


//////////////////////////////////////// server_connect ////////////////////////////////////////////////////

int server_connect(struct server_t* server){

	/* Verificar parâmetro da função e alocação de memória */
	if(server == NULL && server -> ip_port){
		return -1;
    }

	int sockfd;
	char * ip_porta = strdup(server -> ip_port);
	/* Estabelecer ligação ao servidor:

		Preencher estrutura struct sockaddr_in com dados do
		endereço do servidor.

		Criar a socket.

		Estabelecer ligação.
	*/
	struct sockaddr_in* addr;
	if((addr = malloc(sizeof(struct sockaddr_in))) == NULL) {
		fprintf(stderr, "Erro ao alocar memoria! \n");
		free(ip_porta);
		return -1;
	}
	char* token = strtok(ip_porta, ":");

	addr-> sin_family = AF_INET;
	if (inet_pton(AF_INET, token, &(addr-> sin_addr)) < 1) {
		printf("Erro ao converter IP \n");
		free(addr);
		free(ip_porta);
		return -1;
		}

	addr-> sin_port = htons(atoi(strtok(NULL,":")));

	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		fprintf(stderr, "Erro ao criar socket TCP! \n");
		free(ip_porta);
		free(addr);
		return -1;
	}


	// Estabelece conexão com o servidor definido em server
	if (connect(sockfd,(struct sockaddr *)addr, sizeof(*addr)) < 0) {
		fprintf(stderr, "Erro ao conectar-se ao servidor! \n");
		free(ip_porta);
		free(addr);
		close(sockfd);
		return -1;
	}
    server -> socket = sockfd;
    server -> state = 1;

	free(addr);
	free(ip_porta);

	return 0;
}
