#include "common.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>

#include <sys/types.h>
#include <sys/socket.h>

#define BUFSZ 1024

void usage(int argc, char **argv)
{
    printf("usage: ./%s <v4|v6> <server port>\n", argv[0]);
    printf("exemple: ./%s v4 51511\n", argv[0]);
    exit(EXIT_FAILURE);
}

struct client_data
{
    int csock;
    struct sockaddr_storage storage;
};

void * client_thread(void *data)
{
    struct client_data *cdata = (struct client_data *)data;
    struct sockaddr *caddr = (struct sockaddr *)(&cdata->storage);

    char caddrstr[BUFSZ];
    addrtostr(caddr, caddrstr, BUFSZ);
    printf("[log] connection from %s\n", caddrstr);

    char buf[BUFSZ];
    memset(buf, 0, BUFSZ);
    size_t count = recv(cdata->csock, buf, BUFSZ, 0);
    printf("[msg] %s, %d bytes: %s\n", caddrstr, (int)count, buf);

    //sprintf(buf, "remote endpoint %.1000s\n", caddrstr);
    MESSAGE test = {.id_destiny = 0, .id_msg = 0, .id_origen = 0, .payload = "Por favor, fala que funciona!"};
    count = send(cdata->csock, &test, sizeof(test), 0);
    if (count != strlen(buf) + 1)
        logexit("send");
    close(cdata->csock);

    pthread_exit(EXIT_SUCCESS);
}

int main(int argc, char **argv)
{
    if (argc < 3)
        usage(argc, argv);

    struct sockaddr_storage storage;
    if (0 != server_sockaddr_init(argv[1], argv[2], &storage))
        usage(argc, argv);

    int s;
    s = socket(storage.ss_family, SOCK_STREAM, 0);
    if (s == -1)
        logexit("socket");

    int enable = 1;
    if (0 != setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)))
        logexit("setsockopt");

    struct sockaddr *addr = (struct sockaddr *)(&storage);
    if (0 != bind(s, addr, sizeof(storage)))
        logexit("bind");

    if (0 != listen(s, 10)) // 10 -> quantidade máxima de conexões pendentes
        logexit("listen");

    char addrstr[BUFSZ];
    addrtostr(addr, addrstr, BUFSZ);
    printf("bound to %s, waiting connections\n", addrstr);

    while (1)
    {
        struct sockaddr_storage cstorage;
        struct sockaddr *caddr = (struct sockaddr *)(&cstorage);
        socklen_t caddrlen = sizeof(cstorage);

        int csock = accept(s, caddr, &caddrlen);
        if (csock == -1)
            logexit("accept");

        struct client_data *tdata = malloc(sizeof(struct client_data));
        if (!tdata)
            logexit("malloc");

        tdata->csock = csock;
        memcpy(&(tdata->storage), &cstorage, sizeof(cstorage));

        pthread_t tid;
        pthread_create(&tid, NULL, client_thread, tdata);
    }

    exit(EXIT_SUCCESS);
}
