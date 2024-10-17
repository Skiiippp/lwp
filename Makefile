CC = gcc

CFLAGS = -fPIC -Wall -g -O0 -std=gnu99

SOURCES = lwp.c round_robin.c magic64.S 
HEADERS = round_robin.h 

OBJS = lwp.o round_robin.o magic64.o

all: liblwp.so

test: liblwp.so main.c
	$(CC) $(CFLAGS) -o test main.c -I ./include/ -L ./ -llwp

liblwp.so: $(OBJS)
	$(CC) $(CFLAGS) -shared -o $@ $(OBJS)

%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@ -I ./include/

clean:
	rm -rf *.o liblwp.so test

.PHONY: all clean
