#pragma once

#include <stdlib.h>

#include <arpa/inet.h>

typedef struct {
    int id_msg;
    int id_origen;
    int id_destiny;
    char payload[100];
} MESSAGE;

#define BUFSZ sizeof(MESSAGE)

struct client_data
{
    int csock;
    struct sockaddr_storage storage;
    int equipement_id;
};

void logexit(const char *msg);

int addrparse(const char *addrstr, const char *portstr,
              struct sockaddr_storage *storage);

void addrtostr(const struct sockaddr *addr, char *str, size_t strsize);

int server_sockaddr_init(const char *proto, const char *portstr,
                         struct sockaddr_storage *storage);

MESSAGE* get_message_from_input(char aux[100]);