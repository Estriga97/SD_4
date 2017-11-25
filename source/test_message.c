#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <netinet/in.h>

#include "message.h"

void print_message(struct message_t *msg) {
	int i;
	
	printf("\n----- MESSAGE -----\n");
	printf("Tabela número: %d\n", msg->table_num);
	printf("opcode: %d, c_type: %d\n", msg->opcode, msg->c_type);
	switch(msg->c_type) {
		case CT_ENTRY:{
			printf("key: %s\n", msg->content.entry->key);
			printf("datasize: %d\n", msg->content.entry->value->datasize);
		}break;
		case CT_KEY:{
			printf("key: %s\n", msg->content.key);
		}break;
		case CT_KEYS:{
			for(i = 0; msg->content.keys[i] != NULL; i++) {
				printf("key[%d]: %s\n", i, msg->content.keys[i]);
			}
		}break;
		case CT_VALUE:{
			printf("datasize: %d\n", msg->content.data->datasize);
		}break;
		case CT_RESULT:{
			printf("result: %d\n", msg->content.result);
		};
	}
	printf("-------------------\n");
}

int testResult() {
	int result, size, res;
	short opcode, c_type, table_num;
	char *msg_str = NULL;
	struct message_t *msg;

	printf("Módulo mensagem -> teste - Result:");

	msg = (struct message_t *) malloc(sizeof(struct message_t));

	msg->opcode = OC_PUT;
	msg->c_type = CT_RESULT;
	msg->table_num = 7;
	msg->content.result = 1;
	size = message_to_buffer(msg, &msg_str);
	
	opcode = htons(msg->opcode);
	c_type = htons(msg->c_type);
	table_num = htons(msg->table_num);
	res = htonl(msg->content.result);
	result = memcmp(msg_str, &opcode, 2) == 0 &&
		 memcmp(msg_str + 2, &c_type, 2) == 0 && 
		 memcmp(msg_str + 4, &table_num, 2) == 0 &&
		 memcmp(msg_str + 6, &res, 4) == 0;

	free_message(msg);

	msg = buffer_to_message(msg_str, size);

	result = result && msg->opcode == OC_PUT &&
			   msg->c_type == CT_RESULT &&
			   msg->table_num == 7 &&
			   msg->content.result == 1;

	free(msg_str);
	//print_message(msg);
	free_message(msg);

	printf(" %s\n", result ? "passou":"não passou");
	return result;
}

int testKey() {
	int result, size, keysize;
    short keysize_conv, opcode, c_type, table_num;
	char *msg_str = NULL;
	struct message_t *msg;

	printf("Modulo mensagem -> teste - Key:");

	msg = (struct message_t *)malloc(sizeof(struct message_t));
	msg->opcode = OC_PUT;
	msg->c_type = CT_KEY;
	msg->table_num = 8;
	msg->content.key = strdup("abcdef");
	size = message_to_buffer(msg,&msg_str);

	opcode = htons(msg->opcode);
	c_type = htons(msg->c_type);
	table_num = htons(msg->table_num);
	keysize = strlen(msg->content.key);
	keysize_conv = htons((short) keysize);

	char comp_key[keysize];

	memcpy(comp_key, msg_str + 8, keysize);
	result = memcmp(msg_str, &opcode, 2) == 0 &&
		 memcmp(msg_str + 2, &c_type, 2) == 0 &&
		 memcmp(msg_str + 4, &table_num, 2) == 0 &&
		 memcmp(msg_str + 6, &keysize_conv, 2) == 0 &&
		 memcmp(msg_str + 8, &comp_key, keysize) == 0;

	free_message(msg);

	msg = buffer_to_message(msg_str, size);

	result = result && msg->opcode == OC_PUT &&
			   msg->c_type == CT_KEY &&
			   msg->table_num == 8 &&
			   strcmp(msg->content.key, "abcdef") == 0;

	free(msg_str);
	//print_message(msg);
	free_message(msg);

	printf(" %s\n", result ? "passou" : "não passou");
	return result;
}

int testValue() {
	int result, size, datasize, datasize_conv;
	short opcode, c_type, table_num;
	char *msg_str = NULL;
	char *datastr;
	struct message_t *msg;

	printf("Módulo mensagem -> teste - Value:");

	msg = (struct message_t *) malloc(sizeof(struct message_t));
	msg->opcode = OC_PUT;
	msg->c_type = CT_VALUE;
	msg->table_num = 9;
	datastr = strdup("abc");
	msg->content.data = data_create2(strlen("abc") + 1, datastr);
	size = message_to_buffer(msg, &msg_str);

	opcode = htons(msg->opcode);
	c_type = htons(msg->c_type);
	table_num = htons(msg->table_num);
	datasize = msg->content.data->datasize;
	datasize_conv = htonl(datasize);

	char comp_data[datasize];

	memcpy(comp_data, msg_str + 10, datasize);
	result = memcmp(msg_str, &opcode, 2) == 0 &&
		 memcmp(msg_str + 2, &c_type, 2) == 0 &&
		 memcmp(msg_str + 4, &table_num, 2) == 0 &&
		 memcmp(msg_str + 6, &datasize_conv, 4) == 0 &&
		 memcmp(msg_str + 10, &comp_data, datasize) == 0;

	free_message(msg);

	msg = buffer_to_message(msg_str, size);

	result = result && msg->opcode == OC_PUT &&
			   msg->c_type == CT_VALUE &&
			   msg->table_num == 9 &&
			   msg->content.data->datasize == strlen("abc")+1 &&
			   strcmp(msg->content.data->data,"abc") == 0;

	free(msg_str);
	free(datastr);
	//print_message(msg);
	free_message(msg);

	printf(" %s\n", result ? "passou" : "não passou");
	return result;
}

int testEntry() {
	int result, size, keysize, datasize, datasize_conv;
	short opcode, c_type, keysize_conv, table_num;
	char *msg_str = NULL;
	char *datastr = strdup("abc");//malloc1
	struct message_t *msg;
	struct data_t *d;

	printf("Módulo mensagem -> teste - Entry");

	d = data_create2(strlen(datastr) + 1, datastr);//malloc2

	msg = (struct message_t *) malloc(sizeof(struct message_t));//malloc3
	msg->opcode = OC_PUT;
	msg->c_type = CT_ENTRY;
	msg->table_num = 10;
	msg->content.entry = (struct entry_t *) malloc(sizeof(struct entry_t));//malloc4
	msg->content.entry->key = strdup(datastr);//malloc5
	msg->content.entry->value = data_dup(d);//malloc6

	data_destroy(d);//free2

	size = message_to_buffer(msg, &msg_str);
    
	opcode = htons(msg->opcode);
	c_type = htons(msg->c_type);
	table_num = htons(msg->table_num);
	keysize = strlen(msg->content.entry->key);
	keysize_conv = htons(keysize);

	char comp_key[keysize];

	memcpy(comp_key, msg_str + 8, keysize);
	datasize = msg->content.entry->value->datasize;
	datasize_conv = htonl(datasize);

	char comp_data[datasize];

	memcpy(comp_data, msg_str + 6 + 2 + keysize + 4, datasize);

	result = memcmp(msg_str, &opcode, 2) == 0 &&
		 memcmp(msg_str + 2, &c_type, 2) == 0 &&
		 memcmp(msg_str + 4, &table_num, 2) == 0 &&
		 memcmp(msg_str + 6, &keysize_conv, 2) == 0 &&
		 memcmp(msg_str + 8, &comp_key, keysize) == 0 &&
		 memcmp(msg_str + 8 + keysize, &datasize_conv, 4) == 0 &&
		 memcmp(msg_str + 8 + keysize + 4, &comp_data, datasize) == 0;

	free_message(msg);

	msg = buffer_to_message(msg_str, size);

	result = result && msg->opcode == OC_PUT &&
			   msg->c_type == CT_ENTRY &&
			   msg->table_num == 10 &&
			   strcmp(msg->content.entry->key, datastr) == 0 &&
			   msg->content.entry->value->datasize == strlen(datastr) + 1 &&
			   strcmp(msg->content.entry->value->data, datastr) == 0;

	free(msg_str);
	free(datastr);//free1

	//print_message(msg);
	
	free_message(msg);

	printf(" %s\n", result ? "passou" : "não passou");
	return result;
}

int testKeys() {
	int result, size, num_keys, num_keys_conv;
	short opcode, c_type, table_num;
	char *msg_str = NULL;
	struct message_t *msg;
	char *keys[] = {"ul2015", "SD", "teste", "123", NULL};
	short sizes_conv[] = {htons(6), htons(2), htons(5), htons(3)};

	printf("Módulo mensagem -> teste - Keys:");

	msg = (struct message_t *) malloc(sizeof(struct message_t));
	msg->opcode = OC_PUT;
	msg->c_type = CT_KEYS;
	msg->table_num = 12;
	msg->content.keys = (char **) malloc(5 * sizeof(char *));
	msg->content.keys[0] = strdup("ul2015");
	msg->content.keys[1] = strdup("SD");
	msg->content.keys[2] = strdup("teste");
	msg->content.keys[3] = strdup("123");
	msg->content.keys[4] = NULL;

	size = message_to_buffer(msg, &msg_str);

	opcode = htons(msg->opcode);
	c_type = htons(msg->c_type);
	table_num = htons(msg->table_num);
	num_keys = 4;
	num_keys_conv = htonl(num_keys);

	result = memcmp(msg_str, &opcode, 2) == 0 &&
	  	 memcmp(msg_str + 2, &c_type, 2) == 0 &&
		 memcmp(msg_str + 4, &table_num, 2) == 0 &&
	 	 memcmp(msg_str + 6, &num_keys_conv, 4) == 0 &&
		 memcmp(msg_str + 10, &sizes_conv[0], 2) == 0 &&
		 memcmp(msg_str + 12, keys[0], strlen(keys[0])) == 0 &&
		 memcmp(msg_str + 18, &sizes_conv[1], 2) == 0 &&
		 memcmp(msg_str + 20, keys[1], strlen(keys[1])) == 0 &&
		 memcmp(msg_str + 22, &sizes_conv[2], 2) == 0 &&
		 memcmp(msg_str + 24, keys[2], strlen(keys[2])) == 0 &&
		 memcmp(msg_str + 29, &sizes_conv[3], 2) == 0 &&
		 memcmp(msg_str + 31, keys[3], strlen(keys[3])) == 0;

	free_message(msg);

	msg = buffer_to_message(msg_str, size);

	result = result && msg->opcode == OC_PUT &&
			   msg->c_type == CT_KEYS &&
			   msg->table_num == 12 &&
			   strcmp(msg->content.keys[0], keys[0]) == 0 &&
			   strcmp(msg->content.keys[1], keys[1]) == 0 &&
			   strcmp(msg->content.keys[2], keys[2]) == 0 &&
			   strcmp(msg->content.keys[3], keys[3]) == 0 &&
			   msg->content.keys[4] == NULL;

	free(msg_str);
	//print_message(msg);
	free_message(msg);

	printf(" %s\n", result ? "passou" : "não passou");
	return result;
}

int testInvalida() {
	int result;
	char *msg_lixada1 = "1234 50 abcederdfasdasfsdfafadgadgagadgadhgdfhsghsshg";
	struct message_t *msg;

	printf("Módulo mensagem -> teste - Inválida:");

	msg = buffer_to_message(msg_lixada1, strlen(msg_lixada1)+1);

	result = (msg == NULL);

	printf(" %s\n", result ? "passou" : "não passou");
	return result;
}

int main() {
	int score = 0;

	printf("\nIniciando o teste do módulo message\n");

	score += testResult();

	score += testKey();

	score += testValue();

	score += testEntry();

	score += testKeys();

	score += testInvalida();

	printf("Resultados do teste do módulo message: %d em 6\n\n",score);

	return score;
}
