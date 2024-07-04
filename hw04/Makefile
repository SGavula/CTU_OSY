CC = gcc
CFLAGS = -Wall -g -O2

ifndef EXTRA_CFLAGS
EXTRA_CFLAGS = -fsanitize=address -fno-omit-frame-pointer
endif

CFLAGS += $(EXTRA_CFLAGS)

prod-cons: prod-cons.c
	$(CC) -g -lpthread prod-cons.c $(CFLAGS) -o prod-cons

clean:
	rm prod-cons