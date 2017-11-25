CC=gcc
BIN_DIR = binary
TEST_DIR = testes
INC_DIR = include
OBJ_DIR = object
SRC_DIR = source
CFLAGS = -Wall -I $(INC_DIR) -g

all:clean server client

server:$(OBJ_DIR)/data.o $(OBJ_DIR)/entry.o $(OBJ_DIR)/table.o $(OBJ_DIR)/message.o $(OBJ_DIR)/table-server.o $(OBJ_DIR)/table-skel.o
	$(CC)  $(CFLAGS)  $(OBJ_DIR)/entry.o $(OBJ_DIR)/table.o $(OBJ_DIR)/message.o $(OBJ_DIR)/table-$@.o $(OBJ_DIR)/table-skel.o $< -o  $(BIN_DIR)/$@


client:$(OBJ_DIR)/data.o $(OBJ_DIR)/entry.o $(OBJ_DIR)/table.o $(OBJ_DIR)/message.o $(OBJ_DIR)/table-client.o $(OBJ_DIR)/remote-table.o $(OBJ_DIR)/network_client.o
	$(CC)  $(CFLAGS)  $(OBJ_DIR)/entry.o $< $(OBJ_DIR)/table.o $(OBJ_DIR)/message.o $(OBJ_DIR)/table-$@.o $(OBJ_DIR)/remote-table.o $(OBJ_DIR)/network_client.o -o  $(BIN_DIR)/$@


test_table:$(OBJ_DIR)/data.o $(OBJ_DIR)/entry.o $(OBJ_DIR)/table.o $(OBJ_DIR)/test_table.o
	$(CC)  $(CFLAGS)  $(OBJ_DIR)/entry.o $(OBJ_DIR)/$@.o $< $(OBJ_DIR)/table.o -o $(BIN_DIR)/$@
	

test_entry:$(OBJ_DIR)/data.o $(OBJ_DIR)/test_entry.o $(OBJ_DIR)/entry.o
	$(CC)  $(CFLAGS)  $(OBJ_DIR)/$@.o $< $(OBJ_DIR)/entry.o -o $(BIN_DIR)/$@


test_data:$(OBJ_DIR)/data.o $(OBJ_DIR)/test_data.o
	$(CC)  $(CFLAGS)  $(OBJ_DIR)/$@.o $< -o $(BIN_DIR)/$@
		

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f -rf $(OBJ_DIR)/*
	rm -f -rf $(BIN_DIR)/*
