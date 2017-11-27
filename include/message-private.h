#ifndef _MESSAGE_PRIVATE_H
#define _MESSAGE_PRIVATE_H

#include "message.h"

/* Define os possíveis opcodes da mensagem */
#define OC_RT_ERROR	99
#define OC_TABLES 60

/* Define códigos para os possíveis conteúdos da mensagem */
#define CT_SZ_TABLES 60

struct message_t* messgerror ();
void imprimir_resposta(struct message_t *msg);

#endif
