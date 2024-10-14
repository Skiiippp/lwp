CC = gcc

CFLAGS = -Wall -g

SOURCES = lwp.c round_robin.c magic64.S
HEADERS = round_robin.h

OBJS = lwp.o round_robin.o magic64.o

all: liblwp.so

test: liblwp.so test.c
	$(CC) $(CFLAGS) -o test test.c -I ./include/ -L ./lib/ -llwp

liblwp.so: lib/liblwp.so

lib/liblwp.so: $(OBJS)
	$(CC) $(CFLAGS) -shared -o $@ $(OBJS)

%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@ -I ./include/

clean:
	rm -rf *.o lib/liblwp.so test

.PHONY: all clean liblwp.so
