/*
 * exemplo_thread.c
 *
 *  Created on: 30 Nov 2016
 *      Author: Pedro Ferreira
 *
 *  Compilar com: gcc -Wall -pthread exemplo_thread.c -o exemplo_thread
 *
 */

#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

/*****************************************/
struct thread_parameters {
	char *a_string;
	int a_int;
};

/***************************************************************/

int counter = 0, number = 0;

pthread_mutex_t dados = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t	dados_disponiveis = PTHREAD_COND_INITIALIZER;

/***********************************************************************************/

void *thread_main(void *params){
	struct thread_parameters *tp = (struct thread_parameters *) params;
	int *result = 0;

	printf("Nova thread a iniciar\n");
	printf("com dois parâmetros:\n  %s;\n  %d.\n\n", tp->a_string, tp->a_int);

	while (1){

		pthread_mutex_lock(&dados);

		/* Esperar por dados > 0 */
		while (number == 0)
			pthread_cond_wait(&dados_disponiveis, &dados);
	
		printf("  nova thread recebeu dados: %d\n", number);

		sleep((rand() % 5) + 1); /* Fingir atividade */
		counter++;

		number = 0; /* Já processámos dados */

		/* Se já fiz o que tinha a fazer, liberto o mutex*/
		pthread_mutex_unlock(&dados);

		if (counter == tp->a_int) break;
	}

	result = (int *) malloc(sizeof(int));
	printf("Nova thread a terminar\n");
	if (result)
		*result = tp->a_int;
	return result;
}

/********************************************************************/

int main(int argc, char **argv){
	pthread_t nova;
	struct thread_parameters thread_p;
	int *r, res;

	/* Verificar argumentos de entrada */
	if (argc != 3){
		printf("\nNúmero de pârametros inválido\nexemplo_thread <abcd> <10>\n");
		exit(EXIT_FAILURE);
	}

	if (atoi(argv[2]) < 1){
		printf("\nSegundo parâmetro tem de ser >= 1.\n");
		exit(EXIT_FAILURE);
	}

	printf("main() a iniciar\n");

	srand(time(NULL)); /* Nova origem de números aleatórios */

	/* dois parâmetros para a nova thread */
	thread_p.a_string = argv[1];
	thread_p.a_int = atoi(argv[2]);

	/* criação de nova thread */
	if (pthread_create(&nova, NULL, &thread_main, (void *) &thread_p) != 0){
		perror("\nThread não criada.\n");
		exit(EXIT_FAILURE);
	}

	while (1){

		/* Fingir que demorou algum tempo até chegarem dados */
		sleep(rand() % 5);

		pthread_mutex_lock(&dados);

		if (number == 0){ /* Dados anteriores já foram processados */ 

			number = rand() + 1; /* Novos dados > 0 disponíveis */
			printf("main() recebeu e enviou dados: %d\n", number);

			pthread_cond_signal(&dados_disponiveis); /* Avisar que há dados novos > 0 */
		}

		pthread_mutex_unlock(&dados);

		if (counter == thread_p.a_int - 1) break;

	}

	/* Se não precisar de saber quando a thread termina, não preciso fazer join. */
	/* Nesse caso devia fazer pthread_detach depois da thread iniciada. */
	if (pthread_join(nova, (void **) &r) != 0){
		perror("\nErro no join.\n");
		exit(EXIT_FAILURE);
	}

	res = *r;
	free(r);

	printf("main() a terminar\n");

	return res;
}

