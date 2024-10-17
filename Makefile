CC = gcc

CFLAGS = -fPIC -Wall -g -O0 -std=gnu99

SOURCES = lwp.c round_robin.c magic64.S new_rr.c
HEADERS = round_robin.h new_rr.h

OBJS = lwp.o round_robin.o magic64.o new_rr.o

all: liblwp.so

test: liblwp.so test.c
	$(CC) $(CFLAGS) -o test test.c -I ./include/ -L ./ -llwp

liblwp.so: $(OBJS)
	$(CC) $(CFLAGS) -shared -o $@ $(OBJS)

%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@ -I ./include/

clean:
	rm -rf *.o liblwp.so test

.PHONY: all clean
