CFLAGS = -Wall -g
# gcc $(CFLAGS) -c application.c

all:
	gcc $(CFLAGS) -c common.c
	gcc $(CFLAGS) -c application.c
	gcc $(CFLAGS) client.c common.o -o client
	gcc $(CFLAGS) server.c common.o application.o  -o server
