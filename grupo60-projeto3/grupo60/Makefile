INC_DIR = include
SRC_DIR = source
OBJ_DIR = object
LIB_DIR = lib
BIN_DIR = binary

CC = gcc
CFLAGS = -Wall -g -I$(INC_DIR)
LDFLAGS = -L$(LIB_DIR)
LDLIBS = -ltable -lprotobuf-c

EXECS = table_client table_server
LIB = libtable.a

# Lista de todos os arquivos-fonte e objetos
SRCS = $(wildcard $(SRC_DIR)/*.c)
OBJS = $(patsubst $(SRC_DIR)/%.c,$(OBJ_DIR)/%.o,$(SRCS))

all: $(EXECS)

$(LIB_DIR)/$(LIB): $(OBJS)
	ar -rcs $@ $^

table_client: $(OBJ_DIR)/table_client.o $(LIB_DIR)/$(LIB)
	$(CC) -o $(BIN_DIR)/$@ $< $(LDFLAGS) $(LDLIBS)

table_server: $(OBJ_DIR)/table_server.o $(LIB_DIR)/$(LIB)
	$(CC) -o $(BIN_DIR)/$@ $< $(LDFLAGS) $(LDLIBS)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(OBJ_DIR)/* $(BIN_DIR)/* $(LIB_DIR)/*
