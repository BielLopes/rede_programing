CFLAGS = -Wall -g

all:
	gcc $(CFLAGS) -c common.c
	gcc $(CFLAGS) -pthread equipment.c common.o -o equipment
	gcc $(CFLAGS) -pthread server.c common.o -o server

clean:
	rm common.o equipment server