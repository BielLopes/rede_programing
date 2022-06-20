#include "common.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>

#include <sys/types.h>
#include <sys/socket.h>

#define MAXCLIENTS 15
#define THREADSPERCLIENT 2

pthread_t callThd[MAXCLIENTS][THREADSPERCLIENT];
pthread_mutex_t mutexmsg;
int send_message_test = 0;

void usage(int argc, char **argv)
{
    printf("usage: ./%s <v4|v6> <server port>\n", argv[0]);
    printf("exemple: ./%s v4 51511\n", argv[0]);
    exit(EXIT_FAILURE);
}

void * listener_thread(void *data)
{
    struct client_data *cdata = (struct client_data *)data;
    struct sockaddr *caddr = (struct sockaddr *)(&cdata->storage);

    char caddrstr[BUFSZ];
    addrtostr(caddr, caddrstr, BUFSZ);
    printf("[log] connection from %s\n", caddrstr);

    char buf[BUFSZ];
    size_t count;

    sprintf(buf, "remote endpoint %.1000s\n", caddrstr);
    count = send(cdata->csock, buf, strlen(buf) + 1, 0);
    if (count != strlen(buf) + 1)
        logexit("send");

    while(1) {
        memset(buf, 0, BUFSZ);
        count = recv(cdata->csock, buf, BUFSZ, 0);
        printf("[msg] %s, %d bytes: %s\n", caddrstr, (int)count, buf);

        printf("[info] Result strcmpn: %d\n", strcmp(buf, "send"));

        if (0 == strcmp(buf, "send")) {
            pthread_mutex_lock (&mutexmsg);
            send_message_test = 1;
            pthread_mutex_unlock (&mutexmsg);
        }
    }    

    close(cdata->csock);

    pthread_exit(EXIT_SUCCESS);
}

void * talker_thread(void *data)
{
    struct client_data *cdata = (struct client_data *)data;
    struct sockaddr *caddr = (struct sockaddr *)(&cdata->storage);

    char caddrstr[BUFSZ];
    addrtostr(caddr, caddrstr, BUFSZ);

    char buf[BUFSZ];
    size_t count;

    while(1) {
        if (send_message_test == 1) {
            sprintf(buf, "Funcionou porra?\n");
            count = send(cdata->csock, buf, strlen(buf) + 1, 0);
            if (count != strlen(buf) + 1)
                logexit("send");

            pthread_mutex_lock (&mutexmsg);
            send_message_test = 0;
            pthread_mutex_unlock (&mutexmsg);
        }
    }

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

    pthread_mutex_init(&mutexmsg, NULL);

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

        pthread_create(&callThd[0][0], NULL, listener_thread, tdata);
        pthread_create(&callThd[0][1], NULL, talker_thread, tdata);
    }

    pthread_mutex_destroy(&mutexmsg);
    exit(EXIT_SUCCESS);
}
