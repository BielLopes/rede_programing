#include "common.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <time.h>

#include <sys/types.h>
#include <sys/socket.h>

#define THREADSPERCLIENT 2

pthread_t callThd[MAXCLIENTS][THREADSPERCLIENT];
pthread_mutex_t mutexmsg;

SERVER_STORAGE dstorage = {
                            .num_equipment = 0,
                            .ips_available = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
                            .csock_list = {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 1, 1, 1, 1},
                            .requesting_data = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
                            .requesting_from = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
                            .to_delete = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
                            .data_payload = ""
                          };

void usage(int argc, char **argv)
{
    printf("usage: ./%s <v4|v6> <server port>\n", argv[0]);
    printf("exemple: ./%s v4 51511\n", argv[0]);
    exit(EXIT_FAILURE);
}

void * listener_thread(void *data)
{
    struct client_data *cdata = (struct client_data *)data;
    char aux[100];
    sprintf(aux, "%d", cdata->equipement_id+1);
    printf("Equipment %s added\n", aux);

    MESSAGE* buf = NULL;
    size_t count = recv(cdata->csock, buf, BUFSZ, 0);

    buf = malloc(BUFSZ);
    sprintf(aux, "%d", cdata->equipement_id);
    buf->id_msg = 3;
    strcpy(buf->payload, aux);
    count = send(cdata->csock, buf, BUFSZ, 0);
    if (count != BUFSZ)
        logexit("send");

    buf->id_msg = 6;
    strcpy(buf->payload, create_server_ip_list(&dstorage));
    count = send(cdata->csock, buf, BUFSZ, 0);
    if (count != BUFSZ)
        logexit("send");
    free(buf->payload);
    free(buf);

    send_message_broadcast(&dstorage, cdata->equipement_id, 0);

    while(1) {
        count = recv(cdata->csock, buf, BUFSZ, 0);
        switch (buf->id_msg)
        {
        case 2:
            {
                send_message_broadcast(&dstorage, cdata->equipement_id, 1);
                pthread_mutex_lock (&mutexmsg);
                dstorage.num_equipment--;
                dstorage.ips_available[cdata->equipement_id] = 1;
                dstorage.csock_list[cdata->equipement_id] = -1;
                dstorage.to_delete[cdata->equipement_id] = 0;
                pthread_mutex_unlock (&mutexmsg);
                break;
            }
        case 5:
            {
                pthread_mutex_lock (&mutexmsg);
                dstorage.requesting_data[buf->id_origen] = 1;
                dstorage.requesting_from[buf->id_destiny] = 1;
                pthread_mutex_unlock (&mutexmsg);
                break;
            }
        }
        if (buf->id_msg == 2) {
            break;
        }
        
    }    

    close(cdata->csock);

    pthread_exit(EXIT_SUCCESS);
}

void * talker_thread(void *data)
{
    struct client_data *cdata = (struct client_data *)data;

    MESSAGE* buf = malloc(BUFSZ);
    size_t count;
    int data_sended = 0;

    while(1) {
        if (dstorage.requesting_data[cdata->equipement_id] == 1) {
            while (0 == strcmp(dstorage.data_payload, ""))
                continue; // espera até o equipamento escrever os dados

            int request_info_id;
            request_info_id = get_request_info_id(&dstorage);
            buf->id_msg = 6;
            buf->id_origen = request_info_id;
            buf->id_destiny = cdata->equipement_id;
            strcpy(buf->payload, dstorage.data_payload);

            count = send(cdata->csock, buf, BUFSZ, 0);
            if (count != BUFSZ)
                logexit("send");

            pthread_mutex_lock (&mutexmsg);
            dstorage.requesting_data[cdata->equipement_id] = 0;
            dstorage.requesting_from[request_info_id] = 0;
            strcpy(dstorage.data_payload, "");
            pthread_mutex_unlock (&mutexmsg);
            data_sended = 0;

        } 
        if (dstorage.requesting_from[cdata->equipement_id] == 1 && data_sended == 0) {
            buf->id_msg = 5;
            buf->id_origen = cdata->equipement_id;
            count = send(cdata->csock, buf, BUFSZ, 0);
            if (count != BUFSZ)
                logexit("send");
            
            char aux[10];
            srand((unsigned int)time(NULL));
            sprintf(aux, "%.2f ", ((float)rand() / (float)(RAND_MAX)) * 10.);

            pthread_mutex_lock (&mutexmsg);
            strcpy(dstorage.data_payload, aux);
            pthread_mutex_unlock (&mutexmsg);
            data_sended = 1;
        }
        if (dstorage.to_delete[cdata->equipement_id] == 1) {
            pthread_mutex_lock (&mutexmsg);
            dstorage.to_delete[cdata->equipement_id] = 0;
            pthread_mutex_unlock (&mutexmsg);
            break;
        }
    }

    free(buf);

    pthread_exit(EXIT_SUCCESS);
}

int main(int argc, char **argv)
{
    if (argc < 3)
        usage(argc, argv);

    struct sockaddr_storage storage;
    if (0 != server_sockaddr_init(argv[1], argv[2], &storage))
        usage(argc, argv);

    int ssock;
    ssock = socket(storage.ss_family, SOCK_STREAM, 0);
    if (ssock == -1)
        logexit("socket");

    int enable = 1;
    if (0 != setsockopt(ssock, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)))
        logexit("setsockopt");

    struct sockaddr *addr = (struct sockaddr *)(&storage);
    if (0 != bind(ssock, addr, sizeof(storage)))
        logexit("bind");

    if (0 != listen(ssock, 10)) // 10 -> quantidade máxima de conexões pendentes
        logexit("listen");

    // char addrstr[BUFSZ];
    // addrtostr(addr, addrstr, BUFSZ);
    // printf("bound to %s, waiting connections\n", addrstr);

    pthread_mutex_init(&mutexmsg, NULL);

    while (1)
    {
        struct sockaddr_storage cstorage;
        struct sockaddr *caddr = (struct sockaddr *)(&cstorage);
        socklen_t caddrlen = sizeof(cstorage);

        int csock = accept(ssock, caddr, &caddrlen);
        if (csock == -1)
            logexit("accept");

        if (dstorage.num_equipment < MAXCLIENTS) {
            struct client_data *tdata = malloc(sizeof(struct client_data));
            if (!tdata)
                logexit("malloc");

            tdata->csock = csock;
            memcpy(&(tdata->storage), &cstorage, sizeof(cstorage));
            int free_id = get_available_id(&dstorage);
            tdata->equipement_id = free_id;

            pthread_create(&callThd[free_id][0], NULL, listener_thread, tdata);
            pthread_create(&callThd[free_id][1], NULL, talker_thread, tdata);

            dstorage.num_equipment++;
            dstorage.ips_available[free_id] = 1;
            dstorage.csock_list[free_id] = tdata->csock;
        } else {
            MESSAGE* buf = malloc(BUFSZ);
            buf->id_msg = 7;
            strcpy(buf->payload, "Equipment limit exceeded");
            size_t count = send(csock, buf, BUFSZ, 0);
            if (count != BUFSZ)
                logexit("send");
        }
    }

    pthread_mutex_destroy(&mutexmsg);
    exit(EXIT_SUCCESS);
}
