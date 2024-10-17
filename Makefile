CC = gcc

CFLAGS = -fPIC -Wall -g -O0 -std=gnu99

SOURCES = lwp.c round_robin.c magic64.S
HEADERS = lwp.h schedulers.h snakes.h util.h round_robin.h 

OBJS = lwp.o round_robin.o magic64.o

all: liblwp.so

liblwp.so: $(OBJS)
	$(CC) $(CFLAGS) -shared -o $@ $(OBJS)

%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@ -I ./

clean:
	rm -rf *.o liblwp.so test

.PHONY: all clean
