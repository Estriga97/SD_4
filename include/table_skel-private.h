#ifndef _TABLE_SKEL_PRIVATE_H
#define _TABLE_SKEL_PRIVATE_H

void get_keys(int n);

struct message_t* invoke_server_version(struct message_t* msg_pedido);

struct entry_t* get_tbl_keys(int n);

#endif
