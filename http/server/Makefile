# @file Makefile
# @author Ivan Cankov 122199400 <e12219400@student.tuwien.ac.at>
# @date 09.01.2024
#
# @brief Makefile for server

CC = gcc
DEFS = -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L
CFLAGS = -Wall -g -std=c99 -pedantic $(DEFS)
LDFLAGS = -lm

SERVER_TARGET = server
SRC_SERVER = server.c
OBJ_SERVER = $(SRC_SERVER:.c=.o)

.PHONY: all clean

all: $(SERVER_TARGET)

$(SERVER_TARGET): $(OBJ_SERVER)
	$(CC) -o $@ $^ $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f $(SERVER_TARGET) $(OBJ_SERVER)

release:
	tar -cvzf HW3B.tgz server.c Makefile
