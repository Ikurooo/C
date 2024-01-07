CC = gcc
CFLAGS = -Wall -Wextra -pedantic
LDFLAGS = -lm

CLIENT_TARGET = client
SRC_CLIENT = client.c
OBJ_CLIENT = $(SRC_CLIENT:.c=.o)

.PHONY: all clean

all: $(CLIENT_TARGET)

$(CLIENT_TARGET): $(OBJ_CLIENT)
	$(CC) -o $@ $^ $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f $(CLIENT_TARGET) $(OBJ_CLIENT)

release:
	tar -cvzf HW3A.tgz client.c Makefile