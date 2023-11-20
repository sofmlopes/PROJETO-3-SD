BIN_DIR = binary
INC_DIR = include
OBJ_DIR = object
SRC_DIR = source
LIB_DIR = lib
DEP_DIR = dependencies

CC = gcc
CFLAGS = -Wall -MMD -MP -MF $(DEP_DIR)/$*.d -I $(INC_DIR) -g
LIBFLAGS = -L$(LIB_DIR) -l:$(LIB).a -lprotobuf-c
ARFLAGS = -rcs

LIB_OBJ = $(OBJ_DIR)/data.o $(OBJ_DIR)/entry.o $(OBJ_DIR)/list.o $(OBJ_DIR)/table.o
TARGET_OBJ = $(filter-out $(LIB_OBJ), $(wildcard $(OBJ_DIR)/*.o))

LIB = libtable

# Nome dos executáveis
TABLE_CLIENT = $(BIN_DIR)/table_client
TABLE_SERVER = $(BIN_DIR)/table_server

# Fontes e objetos do client
CLIENT_SRC = $(SRC_DIR)/network_client.c $(SRC_DIR)/client_stub.c $(SRC_DIR)/sdmessage.pb-c.c $(SRC_DIR)/table_client.c $(SRC_DIR)/message.c $(SRC_DIR)/stats.c
CLIENT_OBJ = $(patsubst $(SRC_DIR)%.c,$(OBJ_DIR)%.o,$(CLIENT_SRC))

# Fontes e objetos do server
SERVER_SRC = $(SRC_DIR)/network_server.c $(SRC_DIR)/table_skel.c $(SRC_DIR)/sdmessage.pb-c.c $(SRC_DIR)/table_server.c $(SRC_DIR)/message.c $(SRC_DIR)/stats.c $(SRC_DIR)/mutex.c
SERVER_OBJ = $(patsubst $(SRC_DIR)%.c,$(OBJ_DIR)%.o,$(SERVER_SRC))

# Compilar tudo
all: libtable table-client table-server

# Compilar biblioteca
libtable: $(LIB_DIR)/$(LIB).a
$(LIB_DIR)/$(LIB).a: $(LIB_OBJ)
	ar $(ARFLAGS) $@ $^

# Compilar client
table-client: libtable $(CLIENT_OBJ)
	$(CC) $(CFLAGS) -o $(TABLE_CLIENT) $(CLIENT_OBJ) $(LIBFLAGS)

# Compilar server
table-server: libtable $(SERVER_OBJ)
	$(CC) $(CFLAGS) -o $(TABLE_SERVER) $(SERVER_OBJ) $(LIBFLAGS)

# Compilar objetos
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

# Target que define uma dependência do ficheiro objeto sdmessage.pb-c.o relativamente ao ficheiro sdmessage.proto.
$(SRC_DIR)/sdmessage.pb-c.c: sdmessage.proto
	protoc-c --c_out=. sdmessage.proto
	mv sdmessage.pb-c.c $(SRC_DIR)
	mv sdmessage.pb-c.h $(INC_DIR)

include $(wildcard $(DEP_DIR)/*.d)
	
clean:
	rm -rf $(TARGET_OBJ) $(BIN_DIR)/* $(LIB_DIR)/* $(DEP_DIR)/*