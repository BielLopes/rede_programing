#pragma once

#include <stdlib.h>
#include <pthread.h>
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

#define MAXCLIENTS 15

typedef struct {
    int num_equipment;
    int ips_available[MAXCLIENTS];
    int csock_list[MAXCLIENTS];
    int requesting_data[MAXCLIENTS];
    int requesting_from[MAXCLIENTS];
    int to_delete[MAXCLIENTS];
    char data_payload[100];
} SERVER_STORAGE;

int create_message_from_input(MESSAGE* buf, char aux[100], int id);

int get_available_id(const SERVER_STORAGE *dstorage);

int get_request_info_id(const SERVER_STORAGE *dstorage);

void send_message_broadcast(const SERVER_STORAGE* dstorage, int id, int type);

char* create_server_ip_list(const SERVER_STORAGE* dstorage);

void initialize_server_ip_list(int id_list[MAXCLIENTS], char* ids, pthread_mutex_t* mutexlist);

void update_server_ip_list(int id_list[MAXCLIENTS], int id, int type, pthread_mutex_t* mutexlist);

void logexit(const char *msg);

int addrparse(const char *addrstr, const char *portstr,
              struct sockaddr_storage *storage);

int server_sockaddr_init(const char *portstr, struct sockaddr_storage *storage);
