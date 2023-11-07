# @file Makefile
# @author Ivan Cankov 122199400 <e12219400@student.tuwien.ac.at>
# @date 31.10.2023
#
# @brief Makefile for mycompress

CC = gcc
DEFS = -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L
CFLAGS = -Wall -g -std=c99 -pedantic $(DEFS)
LDFLAGS =

OBJECTS = mycompress.o

.PHONY: all clean release

all: mycompress

mycompress: $(OBJECTS)
	$(CC) $(LDFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

mycompress.o: mycompress.c

clean:
	rm -rf *.o mycompress HW1A.tgz

release:
	tar -cvzf HW1A.tgz mycompress.c Makefile
