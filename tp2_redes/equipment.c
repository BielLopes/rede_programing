#include "common.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>

int my_list[MAXCLIENTS] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
int to_exit = 0;
pthread_mutex_t mutexlist;

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

    MESSAGE* buf = malloc(BUFSZ);

    recv(cdata->csock, buf, BUFSZ, 0);
    initialize_server_ip_list(my_list, buf->payload, &mutexlist);

    while(1) {
        recv(cdata->csock, buf, BUFSZ, 0);

        switch (buf->id_msg)
        {
        case 3: // Add equipment
            printf("%s\n", buf->payload);
            update_server_ip_list(my_list, buf->id_destiny, 0, &mutexlist);
            break;
        case 5:
            printf("requested information\n");
            break;
        case 6:
            printf("Value from %d: %s\n", buf->id_origen + 1, buf->payload);
            break;
        case 7: // Target equipment not founded
            printf("%s\n", buf->payload);
            break;
        case 8: // Remove equipment
            printf("%s\n", buf->payload);
            update_server_ip_list(my_list, buf->id_destiny, 1, &mutexlist);
            break;
        }
        if (0 == strcmp(buf->payload, "Successful removal")) {
            break;
        }
    }    

    close(cdata->csock);
    pthread_mutex_lock(&mutexlist);
    to_exit = 1;
    pthread_mutex_unlock(&mutexlist);
    pthread_exit(EXIT_SUCCESS);
}

int main(int argc, char **argv)
{
    if (argc < 3)
        usage(argc, argv);
    
    pthread_mutex_init(&mutexlist, NULL);

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

    MESSAGE* buf = malloc(BUFSZ);
    buf->id_msg = 1;
    size_t count = send(csock, buf, BUFSZ, 0);
    if (count != BUFSZ)
            logexit("send");
    
    recv(csock, buf, BUFSZ, 0);
    printf("New ID: %d\n", atoi(buf->payload) + 1);
    struct client_data *tdata = malloc(sizeof(struct client_data));
    if (!tdata)
        logexit("malloc");

    tdata->csock = csock;
    memcpy(&(tdata->storage), &storage, sizeof(storage));
    tdata->equipement_id = atoi(buf->payload);

    pthread_t tid;
    pthread_create(&tid, NULL, client_thread, tdata);

    while (1)
    {
        char aux[100];
        memset(aux, 0, 100);
        fgets(aux, 100, stdin);
        if (0 == strcmp(aux, "list equipment\n")) {
            for (int i = 0; i < MAXCLIENTS; i++) {
                if (my_list[i] == 1 && i!= tdata->equipement_id) {
                    printf("%d ", i + 1);
                    // printf("Em tese esse é o my_list[%d]!!\n", i);
                }
            }
            printf("\n");
        } else {
            if (0 == create_message_from_input(buf, aux, tdata->equipement_id)) {
                count = send(csock, buf, BUFSZ, 0);
                if (count != BUFSZ)
                    logexit("send");
                if (0 == strcmp(aux, "close connection\n")) {
                    while(to_exit == 0);
                    break;
                }
            } else {
                logexit("create_message_from_input");
            }
        }
    }

    pthread_mutex_destroy(&mutexlist);
    exit(EXIT_SUCCESS);
}