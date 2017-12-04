//Nomes: António Estriga, Diogo Ferreira, Francisco Caeiro
//Numeros: 47839, 47840, 47823
//Grupo: 40

/*
     Programa que implementa um servidor de uma tabela hash com chainning.
     Uso: table-server <port> <table1_size> [<table2_size> ...]
     Exemplo de uso: ./table_server 54321 10 15 20 25
*/

#include "table_server.h"

static int quit = 0;
static int nTables;
static int primario; // 1 = primario, 0 = secundario
static int secundario_ready = 0;

//////////////////////////////////// rtables_sz_tbles ////////////////////////////////////////////////////////

int rtables_sz_tbles(struct server_t *server,char** lst_tbls, int sizE) {
    struct message_t* msg_tables;
    if((msg_tables = (struct message_t*) malloc(sizeof(struct message_t))) == NULL) {
        fprintf(stderr, "Erro ao alocar memoria");
        return -1;
    }
        
    msg_tables -> opcode = OC_SERVER;
    msg_tables -> c_type = CT_SZ_TABLES;
    msg_tables -> table_num = -1;

    char** cnt_keys = msg_tables -> content.keys;
    if((cnt_keys = (char**) malloc(sizeof(char*)*size)) == NULL)
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


    struct message_t msg_resposta = network_send_receive(server, msg_tables);
    if(msg_resposta == NULL){
        imprimir_resposta(messgerror ());
        free(msg_resposta);
        return -2;
    }
    imprimir_resposta(msg_resposta);
    free(msg_resposta);
    return 0;
}

//////////////////////////////////// rtables_ack ////////////////////////////////////////////////////////

int rtables_ack(struct server_t *server) {
    struct message_t* msg_ack;
    if((msg_tables = (struct message_t*) malloc(sizeof(struct message_t))) == NULL) {
        fprintf(stderr, "Erro ao alocar memoria");
        return -1;
    }
        
    msg_ack -> opcode = OC_SERVER;
    msg_ack -> c_type = CT_ACK;
    msg_ack -> table_num = -1;

        
    struct message_t msg_resposta = network_send_receive(server, msg_ack);
    if(msg_resposta == NULL){
        imprimir_resposta(messgerror ());
        free(msg_resposta);
        return -1;
    }
    imprimir_resposta(msg_resposta);
    free(msg_resposta);
    return 0;
}

//////////////////////////////////// network_send_receive ////////////////////////////////////////////////////////

struct message_t *network_send_receive(struct server_t *server, struct message_t *msg){

	/* Verificar parâmetros de entrada */
	if(server == NULL || msg == NULL) {
		fprintf(stderr, "Erro no contudo recebido (server ou msg)!");
		return messgerror ();
	}

	char *message_out;
	int message_size, msg_size, result,i;
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
 	while((result = write_all(server->socket, (char *) &msg_size, _INT)) == -1 && i!= 1){
		sleep(5);
		i++;
	 }
	 i = 0;
	/* Verificar se o envio teve sucesso */
	if(result == -1) {
		fprintf(stderr, "Erro ao enviar!");
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
		fprintf(stderr, "Erro ao enviar!");
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
		sleep(5);
		i++;
	 }
	 i = 0;
	
	if(result == -1) {
		fprintf(stderr, "Erro ao receber o tamanho da mensagem de resposta!");
		free(message_out);
		free_message(msg);
		return messgerror ();
	}

	size = ntohl(size);
	char* buff;
	if((buff = (char*) malloc(size)) == NULL) {
		fprintf(stderr, "Erro ao alocar memoria para a mensagem de resposta!");
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

/////////////////////////////////// rtables_put /////////////////////////////////////////////////////////


int rtables_put(struct rtables_t *rtables, char *key, struct data_t *value){

    if(rtables == NULL || key == NULL || value == NULL){
        fprintf(stderr, "Argumentos NULL");
		return -1;
    }
    
    struct message_t* msg_out;
    if(rtables-> n_tables<=rtables -> table_num){
        imprimir_resposta(messgerror ());
        return -1;
    }

    if((msg_out = (struct message_t*) malloc(sizeof(struct message_t))) == NULL) {
        fprintf(stderr, "Erro ao alocar memoria");
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
        fprintf(stderr, "Erro ao enviar/receber mensagem");
        return -1;
        
    }
    imprimir_resposta(msg_resposta);
    free_message(msg_resposta);
    free_message(msg_out);
    return 0;
}

/////////////////////////////////// rtables_update /////////////////////////////////////////////////////////

int rtables_update(struct rtables_t *rtables, char *key, struct data_t *value){
    struct message_t* msg_out;

    if(rtables == NULL || key == NULL || value == NULL){
        fprintf(stderr, "Argumentos NULL");
		return -1;
    }

    if(rtables-> n_tables<=rtables -> table_num){
        imprimir_resposta(messgerror ());
        return -1;
    }
    if((msg_out = (struct message_t*) malloc(sizeof(struct message_t))) == NULL) {
        fprintf(stderr, "Erro ao alocar memoria");
        return -1;
    }

    msg_out -> opcode = OC_UPDATE;
    msg_out -> c_type = CT_ENTRY;
    msg_out -> table_num = rtables->table_num;
    if((msg_out -> content.entry = (struct entry_t*) malloc(sizeof(struct entry_t))) == NULL) {
        free(msg_out);
        free(msg_out -> content.entry);
        fprintf(stderr, "Erro ao alocar memoria");
        return -1;
    }
    msg_out -> content.entry -> key = strdup(key);
    msg_out -> content.entry -> value = data_dup(value);
    imprimir_resposta(msg_out);

    struct message_t* msg_resposta;
    if((msg_resposta = network_send_receive(rtables-> server, msg_out)) == NULL) {
        fprintf(stderr, "Erro ao enviar/receber mensagem");
        return -1;
        
    }
    imprimir_resposta(msg_resposta);
    free_message(msg_resposta);
    free_message(msg_out);

    return 0;

}

///////////////////////////////////  file_exist  /////////////////////////////////////////////////////////

int file_exist(const char* fl_nm) {
    FILE *file;
    
    if (((file = fopen(fl_name, "r")) != NULL) {
        fclose(file);
        return 1;
    }
    else {
        return 0;
    }
}

///////////////////////////////////  shift  /////////////////////////////////////////////////////////

void shift(struct pollfd* connects, int i) {
    
    if(connects == NULL || i < 0){
        fprintf(stderr, "Argumentos invalidos!");
    }

    int fd;
    short evs;
    short revs;
    
    while(i+1 < SOCKETS_NUMBER && connects[i+1].fd != -1 ) {
        
        fd = connects[i+1].fd;
        evs = connects[i+1].events;
        revs = connects[i+1].revents;

        connects[i+1].fd = -1;
        connects[i+1].events = 0;
        connects[i+1].revents = 0;

        connects[i].fd = fd;
        connects[i].events = evs;
        connects[i].revents = revs;

        i++;
    }

}

///////////////////////////////// make_server_socket ///////////////////////////////////////////////////////////

int make_server_socket(short port){

    int socket_fd;
    struct sockaddr_in server;

    if ((socket_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0 ) {
        fprintf(stderr, "Erro ao criar socket");
        return -1;
    }

    server.sin_family = AF_INET;
    server.sin_port = htons(port);
    server.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(socket_fd, (struct sockaddr *) &server, sizeof(server)) < 0){
        fprintf(stderr, "Erro ao fazer bind!");
        close(socket_fd);
        return -1;
    }

    if (listen(socket_fd, 0) < 0){
        fprintf(stderr, "Erro ao executar listen");
        close(socket_fd);
        return -1;
    }
    int sim = 1;
    if (setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, (int *)&sim, sizeof(sim)) < 0 ) {
        perror("Erro no setsockopt");
    }

    return socket_fd;
}


////////////////////////////////// network_receive_send //////////////////////////////////////////////////////////

/* Função "inversa" da função network_send_receive usada no table-client.
     Neste caso a função implementa um ciclo receive/send:

	Recebe um pedido;
	Aplica o pedido na tabela;
    Envia a resposta.
    
    -1 se o cliente se desconectou, -2 se ocorreu um erro
*/
int network_receive_send(int sockfd){

    /* Verificar parâmetros de entrada */
    if(sockfd == -1) {
        fprintf(stderr, "Erro no socket recebido!");
        return -2;
    }

    char *message_resposta;
    char *message_pedido;
    int msg_length, message_size, msg_size, result;
    struct message_t *msg_pedido, *msg_resposta;


	/* Com a função read_all, receber num inteiro o tamanho da
	     mensagem de pedido que será recebida de seguida.*/
	result = read_all(sockfd, (char *) &msg_size, _INT);

    /* Verificar se a receção teve sucesso */
    if(result <= 0) {
        if(result != 0)
            fprintf(stderr, "Erro no tamanho da mensagem!");
        return -1;
    }

	/* Alocar memória para receber o número de bytes da
         mensagem de pedido. */
 
    msg_length = ntohl(msg_size);     
    if((message_pedido = malloc(msg_length)) == NULL) {
        fprintf(stderr, "Erro ao alocar memoria para a mensagem pedido!");
        return -2;
    }
    
	/* Com a função read_all, receber a mensagem de resposta. */
	result = read_all(sockfd, message_pedido,msg_length);

    /* Verificar se a receção teve sucesso */
    if(result == -1) {
        free(message_pedido);
        fprintf(stderr, "Erro na receção da mensagem resposta!");
        return -1;
    }

	/* Desserializar a mensagem do pedido */
    msg_pedido = buffer_to_message(message_pedido, msg_length);
    imprimir_resposta(msg_pedido);
    /* Verificar se a desserialização teve sucesso */
    if(msg_pedido == NULL) {
        free(message_pedido);
        free_message(msg_pedido);
        fprintf(stderr, "Erro na desserialização!");
        return -2;
    }

    /* Processar a mensagem */
    if(msg_pedido->table_num == -1){
         if((msg_resposta =invoke_server_version(msg_pedido))==NULL)
            return -2;
         if(msg_resposta -> c_type == CT_ACK) {
            secundario_ready = 1;
         }  
    }
    else if(msg_pedido -> table_num >= nTables){
        msg_resposta = messgerror();
        fprintf(stderr, "Erro devido a inexistência de Tabela!");
    }
    else{
        msg_resposta = invoke(msg_pedido);
    }

    imprimir_resposta(msg_resposta);
	/* Serializar a mensagem recebida */
	message_size = message_to_buffer(msg_resposta, &message_resposta);

	/* Verificar se a serialização teve sucesso */
    if(message_size == -1) {
        free(message_pedido);
        free_message(msg_pedido);
        free_message(msg_resposta);
        fprintf(stderr, "Erro na serialização!");
        return -2;
    }

	/* Enviar ao cliente o tamanho da mensagem que será enviada
	     logo de seguida
	*/
    msg_size = htonl(message_size);
 
 	result = write_all(sockfd, (char *) &msg_size, _INT);

    /* Verificar se o envio teve sucesso */
    if(result == -1) {
        free(message_pedido);
        free_message(msg_pedido);
        free_message(msg_resposta);
        free(message_resposta);
        fprintf(stderr, "Erro no envio");
        return -2;
    }

	/* Enviar a mensagem que foi previamente serializada */
	result = write_all(sockfd, message_resposta, message_size);

    /* Verificar se o envio teve sucesso */
    if(result == -1) {
        free(message_pedido);
        free_message(msg_pedido);
        free_message(msg_resposta);
        free(message_resposta);
        fprintf(stderr, "Erro no envio");
        return -2;
    }

	/* Libertar memória */
    free_message(msg_pedido);
    free_message(msg_resposta);
    free(message_resposta);
    free(message_pedido);

	return 0;
}

void quitFunc (){
    quit = 1;

} 

//////////////////////////////////////// server_connect ////////////////////////////////////////////////////

int server_connect(struct server_t* server){

	/* Verificar parâmetro da função e alocação de memória */
	if(server = NULL && server -> ip_port){
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
		return -1;
	}
	char* token = strtok(server -> ip_port, ":");

	addr-> sin_family = AF_INET;
	if (inet_pton(AF_INET, token, &(addr-> sin_addr)) < 1) {
		printf("Erro ao converter IP\n");
		return -1;
		}

	addr-> sin_port = htons(atoi(strtok(NULL,":")));

	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		fprintf(stderr, "Erro ao criar socket TCP!");
		return -1;
	}

	// Estabelece conexão com o servidor definido em server
	if (connect(sockfd,(struct sockaddr *)addr, sizeof(*addr)) < 0) {
		fprintf(stderr, "Erro ao conectar-se ao servidor!");
		close(sockfd);
		return -1;
}
	/* Se a ligação não foi estabelecida, retornar NULL */
    sec_serv -> socket = sockfd;
    sec_serv -> state = 1;

	free(addr);
	return sec_serv;
}

//////////////////////////////////////// pthread_main ////////////////////////////////////////////////////


void* pthread_main(void* params) {
    struct thread_param_t *tp = (struct thread_param_t*) params;
    struct message_t msg_pedido = tp -> msg;
    int* res;
    if((res = (int*) malloc(sizeof(int)) == NULL)) {
        fprintf(stderr, "Erro ao alocar memoria");
        return NULL;
    }
    switch (msg_pedido -> c_type) {
        case CT_PUT:
            if(rtables_put(tp -> rtbl, msg_pedido.content -> key, msg_pedido.content -> data) == -1) {
                fprintf(stderr, "Erro do sec ao fzr o put");
                return NULL;
            }
        break;
        case CT_UPDATE:
            if(rtables_update(tp -> rtbl, msg_pedido.content -> key, msg_pedido.content -> data) == -1) {
                fprintf(stderr, "Erro do sec ao fzr o update");
                return NULL;
            }
        break;
    }
    *res = 0
    return res;

}

//////////////////////////////////////// main ////////////////////////////////////////////////////

int main(int argc, char **argv){

     int listening_socket,i;

    if ((listening_socket = make_server_socket((argv[1]))) < 0) {
        printf("Erro ao criar servidor!");
        return -1;
    }

    if(argc > 2) { //primario
        primario = 1;

        server_t o_server;
        if(o_server = (server_t*) malloc(sizeof(server_t)) == NULL) {
        	fprintf(stderr, "Erro ao preparar o_server!");
        	return -1;
        }
	    
    o_server -> ip_port = strdup(argv[3]);
    o_server -> state = 0; // DOWN

    // criar ficheiro no primario
    fd = fopen("SD_4/ip_secundario","w");
    fprintf(fd,"%lu:%hu",addr-> sin_addr,addr-> sin_port);
    fclose(fp);

    char ** lista_tabelas;

    if((lista_tabelas = (char**) malloc(sizeof(char**)*(argc-1))) == NULL) {
        fprintf(stderr, "Erro ao preparar lista_tabelas!");
        return -1;
    }
    
    for(i = 3; i < argc; i++ ){
        if((lista_tabelas[i-3] = (char *) malloc(strlen(argv[i])+1)) == NULL) {
            while(i != 0) {
                free(lista_tabelas[i-3]);
                i--;
            }
            fprintf(stderr, "Erro ao preparar lista_tabelas[i-2]!");
            return -1;
        }
        memcpy(lista_tabelas[i-3],argv[i],strlen(argv[i])+1);
    }

    lista_tabelas[argc-3] = NULL;
    
    table_skel_init(lista_tabelas);

    int cnt_sec;
	if(server_connect(o_server) < 0)
        cnt_sec = 0;
    else {
        int rtbales_sz_res = rtables_sz_tbles(o_server,lista_tabelas,argc-3);
        if(rtbales_sz_res == -1) {
            return -1;
        }
        else if(rtbales_sz_res == -2)
            secundario -> state = 0;
        else { 
            cnt_sec = 1;
            int rtbales_ack_res = rtables_ack(secundario);
            if(rtbales_ack_res == -1) {
                return -1;

            }
            else if(rtbales_ack_res == -2) {
                secundario -> state = 0;    
            }

        }

    }
    for(i = 0; i < argc-2; i++ ){
        free(lista_tabelas[i]);
    }
    free(lista_tabelas);


}

////////////////////////////////////////////////////////////////

    else if(argc = 2) { //secundario
        primario = 0;
        server_t o_server;
        if(o_server = (server_t*) malloc(sizeof(server_t)) == NULL) {
            fprintf(stderr, "Erro ao preparar o_server!");
            return -1;
        }
        if((o_server -> socket = accept(listening_socket,NULL,NULL)) != -1){
            struct sockaddr_in addr;
            FILE* fd;
            int addr_len = sizeof(addr);
            if(getpeername(o_server -> socket, (struct sockaddr *) &addr, &addr_len)==-1){

            }
            // criar ficheiro no secundario
            fd = fopen("SD_4/ip_primario","w");
            fprintf(fd,"%lu:%hu",addr-> sin_addr,addr-> sin_port);
            fclose(fp);
            while(!secundario_ready){ // secundario_ready ?? estriga?
                network_receive_send(o_server->socket);
            }
        }
        

    }

////////////////////////////////////////////////////////////////

    else {
        printf("Uso: ./server <porta TCP> <table1_size> [<table2_size> ...]\n");
        printf("Exemplo de uso: ./server 54321 10 15 20 25\n");
        return -1;
    }

////////////////////////////////////////////////////////////////

    nTables = argc - 3;
    signal(SIGPIPE, SIG_IGN);

    struct pollfd connections[SOCKETS_NUMBER]; 
    int num_tables = htonl(argc-3);
    int nSockets = 3;
    int net_r_s,res;

    for(i = 0; i < SOCKETS_NUMBER; i++){
        connections[i].fd = -1;
        connections[i].events = 0;
        connections[i].revents = 0;
    }
    connections[0].fd = listening_socket;
    connections[0].events = POLLIN;
    connections[1].fd = fileno(stdin);
    connections[1].events = POLLIN;
    connections[2].fd = o_server -> socket; // tem de ser adaptado??
    connections[2].events = POLLIN;

    while(!quit){ /* espera por dados nos sockets abertos */
        res = poll(connections, nSockets, -1);
        if (res < 0){
            if (errno != EINTR) {
                quit = 1;
            }
            else{
                continue;
                }
        }
        if(connections[0].revents & POLLIN){ /* novo pedido de conexão */
            int socket_de_cliente;
            if(nSockets < SOCKETS_NUMBER){
                if((socket_de_cliente = accept(connections[0].fd,NULL,NULL)) != -1){
                    struct sockaddr_in addr2;
                    int addr_len2 = sizeof(addr2);
                    if(getpeername(socket_client, (struct sockaddr *) &addr2, &addr_len2) == -1) // a confirmar connect_ip
                        return -1;
                    long connect_ip = htonl(atol(strtok(o_server -> ip_port, ":")));
                    if(conect_ip == addr2 -> sin_addr.s_addr) {
                        struct rtables_t *rtables = (struct rtables_t *) malloc(sizeof( struct rtables_t));
                        rtables -> server = o_server; // n tem de ser adaptado?? o server recebido no rtables?
                        rtables -> n_tables = nTables;
                        for(i = 0 ; i < nTables;i++){
                            struct entry* lista_entrys= get_tbl_keys(i);
                            rtables -> table_num = 0;
                            while(*lista_entrys!=NULL){
                                rtables_put(rtables, *lista_entrys.key, *lista_entrys.value); // n ha if
                                lista_entrys++;
                            }
                            rtables -> table_num++;
                        }
                    }

                    printf(" * Client is connected!\n");
                    connections[nSockets].fd = socket_de_cliente;
                    connections[nSockets].events = POLLIN;
                    res = write_all(socket_de_cliente, (char *) &num_tables, _INT); // donde vem este write all?
                    nSockets++;
                }
            }
    }
    /* um dos sockets de ligação tem dados para ler */
        i = 1;
        while(i < SOCKETS_NUMBER && (connections[i].fd != -1 && !quit)) {
            if (connections[i].revents & POLLIN) {
                if(i == 1){ //stdin
                    char input[1000];
                    fgets(input, 1000, stdin);
                    char spliters[] = " ";
                    input[strlen(input)-1] = '\0';
                    char* comando = strtok(input, spliters);
                    
                    if(!strcasecmp(comando,"quit")){
                        quit = 1;
                    }
                    else if(!strcasecmp(comando,"print")){
                        get_keys(atoi(strtok(NULL, spliters)));
                    }
                    else{
                        printf("Comando errado!");
                    }
                }

                else if((net_r_s = network_receive_send(connections[i].fd))== -1){
                    close(connections[i].fd);
                    connections[i].fd = -1;
                    connections[i].events = 0;
                    connections[i].revents = 0;
                    shift(connections,i);
                    nSockets--;
                    printf(" * Client is disconnected!\n");
                    }
                else if(net_r_s == -2){
                    quit = 1; // mudar para mensagem de insucesso
                }
            } 

            if (connections[i].revents & POLLERR ||connections[i].revents & POLLHUP) {
            close(connections[i].fd);
            connections[i].fd = -1;
            connections[i].events = 0;
            connections[i].revents = 0;
            shift(connections,i);
            nSockets--;
            printf(" * Client is disconnected!\n");
            }
            i++;
    }
    }
    table_skel_destroy();
    /* fechar as ligações */
    for(i = 0; i < nSockets; i++){
        close(connections[i].fd);
    }

    return 0;
}


//////////////////////////
int pthread_create (pthread_t*thread,const pthread_attr_t*attr,void* (*func) (void*), void*arg);
