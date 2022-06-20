#include "common.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>

void usage(int argc, char **argv)
{
    if (argc >= 4)
    {
        printf("usage: ./%s <server IP> <server port>\n", argv[0]);
        printf("exemple: ./%s 127.0.0.1 51511\n", argv[0]);
    }
    exit(EXIT_FAILURE);
}

void * client_thread(void *data)
{
    struct client_data *cdata = (struct client_data *)data;

    char buf[BUFSZ];

    while(1) {
        memset(buf, 0, BUFSZ);
        recv(cdata->csock, buf, BUFSZ, 0);

        printf("[msg] recived from server: %s\n", buf);
    }    

    close(cdata->csock);

    pthread_exit(EXIT_SUCCESS);
}

int main(int argc, char **argv)
{
    if (argc < 3)
        usage(argc, argv);

    struct sockaddr_storage storage;
    if (0 != addrparse(argv[1], argv[2], &storage))
        usage(argc, argv);

    int csock;
    csock = socket(storage.ss_family, SOCK_STREAM, 0);
    if (csock == -1)
        logexit("socket");

    struct sockaddr *addr = (struct sockaddr *)(&storage);
    if (0 != connect(csock, addr, sizeof(storage)))
        logexit("connect");

    char buf[BUFSZ];

    struct client_data *tdata = malloc(sizeof(struct client_data));
    if (!tdata)
        logexit("malloc");

    tdata->csock = csock;
    memcpy(&(tdata->storage), &storage, sizeof(storage));

    pthread_t tid;
    pthread_create(&tid, NULL, client_thread, tdata);

    while (1)
    {
        memset(buf, 0, BUFSZ); // Clean Buffer

        // Read from keyboard the next command to server
        printf("> ");
        fgets(buf, BUFSZ - 1, stdin);
        size_t count = send(csock, buf, strlen(buf)-1, 0);
        if (count != strlen(buf)-1)
            logexit("send");
    }

    close(csock);

    exit(EXIT_SUCCESS);
}