# @file Makefile
# @author Ivan Cankov 122199400 <e12219400@student.tuwien.ac.at>
# @date 07.01.2024
#
# @brief Makefile for client

CC = gcc
DEFS = -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L
CFLAGS = -Wall -g -std=c99 -pedantic $(DEFS)
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