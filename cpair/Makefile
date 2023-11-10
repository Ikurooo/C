# @file Makefile
# @author Ivan Cankov 122199400 <e12219400@student.tuwien.ac.at>
# @date 31.10.2023
#
# @brief Makefile for cpair

CC = gcc
DEFS = -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L
CFLAGS = -Wall -g -std=c99 -pedantic $(DEFS)
LDFLAGS = -lm

OBJECTS = cpair.o

.PHONY: all clean release

all: cpair

cpair: $(OBJECTS)
	$(CC) -o $@ $^ $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

cpair.o: cpair.c

clean:
	rm -rf *.o cpair HW1A.tgz

release:
	tar -cvzf HW1A.tgz cpair.c Makefile

