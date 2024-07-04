CC = gcc
CFLAGS = -Wall -g -O2

ifndef EXTRA_CFLAGS
EXTRA_CFLAGS = -fsanitize=address -fno-omit-frame-pointer
endif

CFLAGS += $(EXTRA_CFLAGS)

factory: factory.c
	$(CC) -g -lpthread factory.c $(CFLAGS) -o factory

clean:
	rm factory
