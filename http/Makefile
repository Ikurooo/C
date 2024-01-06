CC = gcc
CFLAGS = -Wall -Wextra -pedantic
LDFLAGS = -lm

SERVER_TARGET = server
CLIENT_TARGET = client

SRC_SERVER = server.c
SRC_CLIENT = client.c

OBJ_SERVER = $(SRC_SERVER:.c=.o)
OBJ_CLIENT = $(SRC_CLIENT:.c=.o)

.PHONY: all clean

all: $(SERVER_TARGET) $(CLIENT_TARGET)

$(SERVER_TARGET): $(OBJ_SERVER)
	$(CC) $(LDFLAGS) -o $@ $^

$(CLIENT_TARGET): $(OBJ_CLIENT)
	$(CC) $(LDFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f $(SERVER_TARGET) $(CLIENT_TARGET) $(OBJ_SERVER) $(OBJ_CLIENT)
