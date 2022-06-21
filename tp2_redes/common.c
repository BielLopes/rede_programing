#include "common.h"

#include <inttypes.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>

void logexit(const char *msg)
{
    perror(msg);
    exit(EXIT_FAILURE);
}

int addrparse(const char *addrstr, const char *portstr,
              struct sockaddr_storage *storage)
{
    if (addrstr == NULL || portstr == NULL)
        return -1;

    uint16_t port = (uint16_t)atoi(portstr); // unsigned short -> Internet partner for port number
    if (port == 0)
        return -1;
    port = htons(port); // htons -> host to network short

    struct in_addr inaddr4; // IPv4 - 32 bit IP adress
    if (inet_pton(AF_INET, addrstr, &inaddr4))
    {
        struct sockaddr_in *addr4 = (struct sockaddr_in *)storage;
        addr4->sin_family = AF_INET;
        addr4->sin_port = port;
        addr4->sin_addr = inaddr4;
        return 0;
    }

    struct in6_addr inaddr6; // IPv6 - 128 bit IP adress
    if (inet_pton(AF_INET6, addrstr, &inaddr6))
    {
        struct sockaddr_in6 *addr6 = (struct sockaddr_in6 *)storage;
        addr6->sin6_family = AF_INET6;
        addr6->sin6_port = port;
        // addr6->sin6_addr = inaddr6;
        memcpy(&(addr6->sin6_addr), &inaddr6, sizeof(inaddr6));
        return 0;
    }

    return -1;
}

void addrtostr(const struct sockaddr *addr, char *str, size_t strsize)
{
    int version;
    char addrstr[INET6_ADDRSTRLEN + 1] = "";
    uint16_t port;

    if (addr->sa_family == AF_INET)
    {
        version = 4;
        struct sockaddr_in *addr4 = (struct sockaddr_in *)addr;
        if (!inet_ntop(AF_INET, &(addr4->sin_addr), addrstr,
                       INET6_ADDRSTRLEN + 1))
            logexit("ntop");
        port = ntohs(addr4->sin_port); // network to short
    }
    else if (addr->sa_family == AF_INET6)
    {
        version = 6;
        struct sockaddr_in6 *addr6 = (struct sockaddr_in6 *)addr;
        if (!inet_ntop(AF_INET6, &(addr6->sin6_addr), addrstr,
                       INET6_ADDRSTRLEN + 1))
            logexit("ntop");
        port = ntohs(addr6->sin6_port); // network to short
    }
    else
    {
        logexit("unknown protocol family");
    }

    if (str)
        snprintf(str, strsize, "IPv%d %s %hu", version, addrstr, port);
}

int server_sockaddr_init(const char *proto, const char *portstr,
                         struct sockaddr_storage *storage)
{
    uint16_t port = (uint16_t)atoi(portstr); // unsigned short -> Internet partner for port number
    if (port == 0)
        return -1;
    port = htons(port); // htons -> host to network short

    memset(storage, 0, sizeof(*storage));
    if (0 == strcmp(proto, "v4"))
    {
        struct sockaddr_in *addr4 = (struct sockaddr_in *)storage;
        addr4->sin_family = AF_INET;
        addr4->sin_addr.s_addr = INADDR_ANY;
        addr4->sin_port = port;
    }
    else if (0 == strcmp(proto, "v6"))
    {
        struct sockaddr_in6 *addr6 = (struct sockaddr_in6 *)storage;
        addr6->sin6_family = AF_INET6;
        addr6->sin6_port = port;
        addr6->sin6_addr = in6addr_any;
    }
    else
    {
        return -1;
    }

    return 0;
}

void create_message_from_input(MESSAGE* buf, char aux[100], int id) {
    if (0 == strcmp(aux, "close connectiont")) {
        buf->id_msg = 2;
        buf->id_origen = id;
    } else {
        char* token = strtok(aux, " ");
        for (int i = 0; i < 4; i++) {
            token = strtok(NULL, " ");
        }

        buf->id_msg = 5;
        buf->id_origen = id;
        buf->id_destiny = atoi(token);
    }
}

int get_available_id(const SERVER_STORAGE *dstorage) {
    int indice;
    for (indice = 0; indice < MAXCLIENTS; indice++) {
        if (dstorage->ips_available[indice] == 0)
            break;
    }
    return indice;
}

int get_request_info_id(const SERVER_STORAGE *dstorage) {
    int indice;
    for (indice = 0; indice < MAXCLIENTS; indice++)
        if (dstorage->requesting_from[indice] == 1)
            break;

    return indice;
}

void send_message_broadcast(const SERVER_STORAGE* dstorage, int id, int type) {
    // Type 0-> add; Type 1-> remove; 
    MESSAGE* buf = malloc(BUFSZ);
    buf->id_destiny = id;
    char aux[100];
    if (type) {
        sprintf(aux, "Equipment %d removed", id);
        buf->id_msg = 8;
    } else {
        sprintf(aux, "Equipment %d added", id);
        buf->id_msg = 3;
    }
    strcpy(buf->payload, aux);


    if (type)
    for (int i = 0; i < MAXCLIENTS; i++) {
        if (dstorage->ips_available[i] == 0 && i != id) {
            size_t count = send(dstorage->csock_list[i], buf, BUFSZ, 0);
            if (count != BUFSZ)
                logexit("send");
        }
    }
}

char* create_server_ip_list(const SERVER_STORAGE* dstorage) {
    char* list = malloc(sizeof(char)*100);
    char aux[5];
    int started = 0;
    for (int i = 0; i < MAXCLIENTS; i++) {
        if (dstorage->ips_available[i] == 0) {
            if (started == 0) {
                memset(list, 0, 100);
                sprintf(list, "%d", i);
                started = 1;
            } else {
                memset(aux, 0, 5);
                sprintf(aux, " %d", i);
                strcat(list, aux);
            }
        }
    }
    return list;
}

void initialize_server_ip_list(int id_list[MAXCLIENTS], char* ids, pthread_mutex_t* mutexlist) {
    char* token = strtok(ids, " ");
    int indice = 0;
    int list_indice[MAXCLIENTS];
    while (token != NULL) {
        list_indice[indice] = atoi(token);
        token = strtok(NULL, " ");
        indice++;
    }
    indice = 0;
    for (int i = 0; i < MAXCLIENTS; i++) {
        for (int k = 0; k < MAXCLIENTS; k++) {
            if (list_indice[indice] == k) {
                pthread_mutex_lock(mutexlist);
                id_list[k] = 1;
                pthread_mutex_unlock(mutexlist);
                indice++;
                break;
            }
        }
    }
}

void update_server_ip_list(int id_list[MAXCLIENTS], int id, int type, pthread_mutex_t* mutexlist) {
    // Type 0-> add; Type 1-> remove;
    if (type) {
        pthread_mutex_lock(mutexlist);
        id_list[id] = 0;
        pthread_mutex_unlock(mutexlist);
    } else {
        pthread_mutex_lock(mutexlist);
        id_list[id] = 1;
        pthread_mutex_unlock(mutexlist);
    }
}
