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
pthread_mutex_t dados = PTHREAD_MUTEX_INITIALIZER;

///////////////////////////////////  file_exist  /////////////////////////////////////////////////////////

int file_exist(const char* fl_nm) {
    FILE *file;
    
    if ((file = fopen(fl_nm, "r")) != NULL) {
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

///////////////////////////////////  prt_wrong_args  /////////////////////////////////////////////////////////

void prt_wrong_args() {
    printf("Uso de server primario: ./server <porta TCP> <IpSecundario:porta TCP> <table1_size> [<table2_size> ...]\n");
    printf("Exemplo de uso: ./server 54321 127.0.0.1:54322 10 15 20 25\n");
    printf("Uso de server secundario: ./server <porta TCP> \n");
    printf("Exemplo de uso: ./server 54321 \n");
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

    o argumento int* ack serve para receber um ack    
    
    -1 se o cliente se desconectou, -2 se ocorreu um erro
*/
int network_receive_send(int sockfd, int* ack){

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
         if((msg_resposta = invoke_server_version(msg_pedido))==NULL)
            return -2;
         if(msg_resposta -> opcode == (OC_ACK + 1)) {
            *ack = 1;
         }
    }
    else if(msg_pedido -> table_num >= nTables){
        msg_resposta = messgerror();
        fprintf(stderr, "Erro devido a inexistência de Tabela!");
    }
    else{
        msg_resposta = invoke(msg_pedido); //*
        
        if(ack!=NULL){
        pthread_t nova;
        struct thread_param_t pthread_p;
        pthread_p.msg = msg_pedido;

        struct server_t* server;
        int* r;
        if((server = (struct server_t*) malloc(sizeof(struct server_t))) == NULL) {
            fprintf(stderr, "Erro ao alocar memoria");
        }
        if((r = (int*) malloc(sizeof(int))) == NULL) {
            fprintf(stderr, "Erro ao alocar memoria");
        }
        server -> socket = sockfd;
        
        pthread_p.msg = msg_pedido;
        pthread_p.server = server;
        pthread_p.table_num = nTables;

        if (pthread_create(&nova, NULL, &pthread_main, (void *) &pthread_p) != 0){
		    fprintf(stderr, "Thread não criada.");
        }

        if (pthread_join(nova, (void **) &r) != 0){
		    fprintf(stderr, "Erro no join.");
        }
        if(*r != 0) {
            *ack = 1;
        }

        free(server);
        free(r);

    }}

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

//////////////////////////////////////// pthread_main ////////////////////////////////////////////////////


void* pthread_main(void* params) {
    struct thread_param_t *tp = (struct thread_param_t*) params;
    struct message_t* msg_pedido = tp -> msg;
    int* res;
  
    if((res = (int*) malloc(sizeof(int))) == NULL) {
        fprintf(stderr, "Erro ao alocar memoria");
        return NULL;
    }
    pthread_mutex_lock(&dados);
    switch (msg_pedido -> opcode) {
        case OC_PUT:
            if(rtables_put(tp -> server,tp->table_num, msg_pedido -> content.entry->key, msg_pedido -> content.entry->value) == -1) {
                fprintf(stderr, "Erro do secundario ao fazer o put");
                return NULL;
            }
        break;
        case OC_UPDATE:
            if(rtables_update(tp -> server,tp->table_num, msg_pedido -> content.entry->key, msg_pedido -> content.entry->value) == -1) {
                fprintf(stderr, "Erro do secundario ao fazer o update");
                return NULL;
            }
        break;
    }
    pthread_mutex_unlock(&dados);
    *res = 0;
    
    return res;

}

//////////////////////////////////////// main ////////////////////////////////////////////////////

int main(int argc, char **argv){
    printf("%s",argv[0]);
    fflush(stdout);
    char ** lista_tabelas;
    int listening_socket,i;
    struct server_t* o_server;
    int fl_exist = (file_exist(FILE_PATH_1) || file_exist(FILE_PATH_2));  //ver se complia

    int* ack;
    if((ack = (int*) malloc(sizeof(int))) == NULL) {
        fprintf(stderr, "Erro ao alocar memoria");
    }
    if((o_server = (struct server_t*) malloc(sizeof(struct server_t))) == NULL) { // essensial!
        fprintf(stderr, "Erro ao preparar o_server!");
        return -1;
    }

    if ((listening_socket = make_server_socket(atoi(argv[1]))) < 0) { // essensial!
        printf("Erro ao criar servidor!");
        return -1;
    }
    if(argc > 2 && !fl_exist) { //primario
        primario = 1;

        if((o_server -> ip_port = strdup(argv[2])) == NULL) {
            fprintf(stderr, "Erro ao alocar memoria");
            return -1;
        }
        o_server -> state = 0; // DOWN


        FILE* fd;
        // criar ficheiro no primario
        if((fd = fopen(FILE_PATH_1,"w")) == NULL) { // essensial!
            fprintf(stderr, "Erro ao criar ficheiro");
            return -1;
        }
        fprintf(fd,"%s", o_server -> ip_port);
        fclose(fd);


        if((lista_tabelas = (char**) malloc(sizeof(char**)*(argc-1))) == NULL) { // essensial!
            fprintf(stderr, "Erro ao preparar lista_tabelas!");
            return -1;
        }
        
        for(i = 3; i < argc; i++ ){
            if((lista_tabelas[i-3] = (char *) malloc(strlen(argv[i])+1)) == NULL) { // essensial!
                while(i != 0) {
                    free(lista_tabelas[i-3]);
                    i--;
                }
                free(o_server);
                free(ack);
                fprintf(stderr, "Erro ao preparar lista_tabelas[i-2]!");
                return -1;
            }
            memcpy(lista_tabelas[i-3],argv[i],strlen(argv[i])+1);
        }

        lista_tabelas[argc-3] = NULL; 
        
        if((table_skel_init(lista_tabelas)) == -1) { // essensial!
            fprintf(stderr, "Erro ao criar tabelas");
            return -1;
        }
        if(server_connect(o_server) < 0)
            o_server -> state = 0;
        else {
            o_server -> state = 1;
            if((rtables_sz_tbles(o_server,lista_tabelas,argc-3)) == -1) {
                o_server -> state = 0;
            }
            else {
                if((rtables_ack(o_server)) == -1) {
                    o_server -> state = 0;
                }
            }
        }

    }

////////////////////////////////////////////////////////////////

    else if(argc == 2 && !fl_exist){ //secundario
        primario = 0;
        if((o_server -> socket = accept(listening_socket,NULL,NULL)) != -1){
            struct sockaddr_in* addr;
            int addr_len = sizeof(struct sockaddr_in);
            if((addr = (struct sockaddr_in*) malloc(addr_len)) == NULL) {  // essensial!
                fprintf(stderr, "Erro ao alocar memoria");
                return -1;
            }
            /*
            FILE* fd;
            if((getpeername(o_server -> socket, (struct sockaddr *) &addr,(socklen_t *)  &addr_len)) ==-1){
                fprintf(stderr, "Erro ao encontrar address primario por falta de recursos");
                return -1;
            }
            else {

                char* ip;
                if((ip = (char*) malloc(MAX_READ)) == NULL) {
                    fprintf(stderr, "Erro ao alocar memoria");
                    return -1;
                }
                if((o_server -> ip_port = (char*) malloc(MAX_READ)) == NULL) {
                    fprintf(stderr, "Erro ao alocar memoria");
                    return -1;
                }
                if((ip=inet_ntoa(addr -> sin_addr)) == 0) {
                    fprintf(stderr, "Erro ao preparar IP");
                    return -1;
                }
                sprintf(o_server -> ip_port,"%s:%hu",ip ,ntohs(addr-> sin_port));
                o_server -> state = 1;
                free(ip);
            }

            // criar ficheiro no secundario sobre o primario
            if((fd = fopen(FILE_PATH_2,"w")) == NULL) { // essensial!
                fprintf(stderr, "Erro ao criar ficheiro");
                return -1;
            }
            fprintf(fd,"%s", o_server -> ip_port);
            fclose(fd);*/
           
            *ack = 0;

            while(!*ack) {
                if((network_receive_send(o_server->socket, ack)) < 0){//servidor secundario sem tabelas
                    fprintf(stderr, "Erro ao atualizar tabelas!");
                    free(o_server -> ip_port);
                    free(o_server);
                    free(ack);
                    free(addr);
                    remove(FILE_PATH_2);
                    return -1;   
                }
            }

            free(addr);
            
        }

    }

////////////////////////////////////////////////////////////////

    else if(fl_exist) { //recuperação (como secundario)
        FILE* fd;
        char buff_read [MAX_READ];
        if(argc  > 2) {
            if((fd = fopen(FILE_PATH_1,"r")) == NULL) { // essensial!
                fprintf(stderr, "Erro ao encontrar ficheiro");
                return -1;
            }
        }
        else if(argc == 2){
            if((fd = fopen(FILE_PATH_2,"r")) == NULL) { // essensial!
                fprintf(stderr, "Erro ao encontrar ficheiro");
                return -1;
            }
        }

        else { // má introdução de argumentos
            prt_wrong_args();
            return -1;
        }

        if((fgets(buff_read, MAX_READ, fd)) == NULL) {
            fprintf(stderr, "Erro no acesso ao ficheiro"); // essencial
            return -1;
        }

        fclose(fd);
    
        buff_read[strlen(buff_read)-1] = '\0';

        o_server -> ip_port = strdup(buff_read);
        o_server -> state = 1;

        if(update_state(o_server) < 0) // again, o q fzmos neste caso em q ele n consegue updatar as tabelas?
            o_server -> state = 0; //*
        else {
            o_server -> state = 1;
            }
        }

////////////////////////////////////////////////////////////////

    else { // má introdução de argumentos
        prt_wrong_args();
        return -1;
    }

//////////////////////// main de ambos (aqui começa a ação) ///////////////////////////////////

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
    connections[1].fd = o_server -> socket; // tem de ser adaptado??
    connections[1].events = POLLIN;
    connections[2].fd = fileno(stdin);
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
            int socket_client;
            if(nSockets < SOCKETS_NUMBER){
                if((socket_client = accept(connections[0].fd,NULL,NULL)) != -1){
                    struct sockaddr_in addr2;
                    int addr_len2 = sizeof(addr2);
                    if(getpeername(socket_client, (struct sockaddr *) &addr2,(socklen_t *) &addr_len2) == -1) { // a confirmar connect_ip, e fzr msm merda q tá em cima?
                        fprintf(stderr, "Erro ao encontrar cliente");
                        continue;
                    }
                    long connect_ip = htonl(atol(strtok(o_server -> ip_port, ":")));
                    if(connect_ip == addr2.sin_addr.s_addr) {//server secundario
                        o_server->socket = socket_client;
                        if(rtables_sz_tbles(o_server,lista_tabelas,argc-3) == -1) {
                             o_server -> state = 0;
                        }
                        for(i = 0 ; i < nTables;i++){
                            struct entry_t* lista_entrys = get_tbl_keys(i);
                            while(lista_entrys -> key != NULL){
                                rtables_put(o_server,i, (*lista_entrys).key, (*lista_entrys).value); // n ha if, TODO
                                lista_entrys++;
                            }
                        }
                    }else{//cliente
                    printf(" * Client is connected!\n");
                    connections[nSockets].fd = socket_client;
                    connections[nSockets].events = POLLIN;
                    res = write_all(socket_client, (char *) &num_tables, _INT);
                    nSockets++;}
                }
                else {
                    fprintf(stderr, "Erro ao aceitar client");
                }
            }
    }
    /* um dos sockets de ligação tem dados para ler */
        if(primario)
            i = 2;
        else{
            i = 1;
        }
        while(i < SOCKETS_NUMBER && (connections[i].fd != -1 && !quit)) {
            if (connections[i].revents & POLLIN) {
                if(i == 2){ //stdin
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
                else{
                    *ack = 0;
                    if(!primario && i!=2){
                        primario = 1;
                        o_server -> state = 0;
                        }    
                    if((net_r_s = network_receive_send(connections[i].fd, o_server->state?ack:NULL)) == -1){
                        close(connections[i].fd);
                        connections[i].fd = -1;
                        connections[i].events = 0;
                        connections[i].revents = 0;
                        shift(connections,i);
                        nSockets--;
                        printf(" * Client is disconnected!\n");
                    }
                    else if(net_r_s == -2){
                        fprintf(stderr, "Operação falhou"); //*
                    }
                    if(ack != 0) {
                        o_server -> state = 0;
                    }
                    free(ack);
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
     // libertação das tabelas usadas para passar a informação para o secundario
    if(argc>2){
    for(i = 0; i < argc-2; i++ ){
        free(lista_tabelas[i]);
    }
    free(lista_tabelas);}
        
    free(o_server -> ip_port);
    free(o_server);
    free(ack);

    return 0;
}
