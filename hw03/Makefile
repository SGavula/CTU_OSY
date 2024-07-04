CC=gcc
CFLAGS=-Wall

ifdef EXTRA_CFLAGS
CFLAGS+= $(EXTRA_CFLAGS)
endif

all: forkpipe nsd

forkpipe: forkpipe.c
	$(CC) forkpipe.c $(CFLAGS) -o forkpipe

nsd: nsd_main.o nsd.o nd.o nsd.h nd.h
	$(CC) $(CFLAGS) -o nsd nsd_main.o nsd.o nd.o

nsd_main.o: nsd_main.c
	$(CC) $(CFLAGS) -c nsd_main.c

nsd.o: nsd.c
	$(CC) $(CFLAGS) -c nsd.c

nd.o: nd.c
	$(CC) $(CFLAGS) -c nd.c

clean:
	rm *.o nsd forkpipe
