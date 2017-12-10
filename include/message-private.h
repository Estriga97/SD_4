#ifndef _MESSAGE_PRIVATE_H
#define _MESSAGE_PRIVATE_H

#include "message.h"

/* Define os possíveis opcodes da mensagem */
#define OC_RT_ERROR	99
#define OC_SZ_TABLES 60
#define OC_ACK 70
#define OC_IP_PORT 80
#define OC_HELLO 90



/* Define códigos para os possíveis conteúdos da mensagem */

/* Função que garante o envio de len bytes armazenados em buf,
   através da socket sock.
*/
int write_all(int sock, char *buf, int len);

/* Função que garante a receção de len bytes através da socket sock,
   armazenando-os em buf.
*/
int read_all(int sock, char *buf, int len);

struct message_t* messgerror ();
void imprimir_resposta(struct message_t *msg);



#endif
