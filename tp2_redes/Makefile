CFLAGS = -Wall -g

all:
	gcc $(CFLAGS) -c common.c
	gcc $(CFLAGS) -pthread client.c common.o -o client
	gcc $(CFLAGS) -pthread server.c common.o -o server

clean:
	rm common.o client server