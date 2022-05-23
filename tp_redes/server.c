#include "common.h"
#include "application.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <sys/types.h>

void usage(int argc, char **argv)
{
    if (argc >= 4)
    {
        printf("usage: ./%s <v4|v6> <server port>\n", argv[0]);
        printf("exemple: ./%s v4 51511\n", argv[0]);
    }
    exit(EXIT_FAILURE);
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

    while (1)
    {
        struct sockaddr_storage cstorage;
        struct sockaddr *caddr = (struct sockaddr *)(&cstorage);
        socklen_t caddrlen = sizeof(cstorage);

        int csock = accept(s, caddr, &caddrlen);
        if (csock == -1)
            logexit("accept");

        char buf[BUFSZ];

        struct storage_communication storage = {.equipments = {{0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}}, .total_equipment = 0};

        while (1)
        {
            // Recive message and create struct to represent them
            memset(buf, 0, BUFSZ);
            size_t count = recv(csock, buf, BUFSZ, 0);
            struct message_type message = {.command = -1, .equipment = -1, .lis_sensors = {-1, -1, -1, -1}, .list_sensors_length = 0};
            int opcode = validate_and_create_message(buf, &message);

            char *response = NULL;

            switch (opcode)
            {
            case -1:
            {
                close(csock);
                logexit("validate_and_create_message");
                break;
            }
            case 1:
            {
                response = (char *)malloc(sizeof(char) * BUFSZ);
                strcpy(response, "invalid sensor");
                break;
            }
            case 2:
            {
                response = (char *)malloc(sizeof(char) * BUFSZ);
                strcpy(response, "invalid equipment");;
                break;
            }
            case 0:
            {
                response = update_bd_and_create_response(&message, &storage);
                break;
            }
            default:
                break;
            }

            memset(buf, 0, BUFSZ);
            strcpy(buf, response);
            free(response);
            count = send(csock, buf, strlen(buf), 0);
            if (count != strlen(buf))
                logexit("send");
        }

        close(csock);
    }

    exit(EXIT_SUCCESS);
}