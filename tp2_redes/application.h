#pragma once

#define MAXCLIENTS 15

typedef struct {
    int num_equipment;
    int ips_available[MAXCLIENTS];
    int csock_list[MAXCLIENTS];
    int requesting_data[MAXCLIENTS];
    int requesting_from[MAXCLIENTS];
    char data_payload[100];
} SERVER_STORAGE;

int get_available_id(const SERVER_STORAGE *dstorage);

int get_request_info_id(const SERVER_STORAGE *dstorage);