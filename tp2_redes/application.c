#include "application.h"

int get_available_id(const SERVER_STORAGE *dstorage) {
    int indice;
    for (indice = 0; indice < MAXCLIENTS; indice++) {
        if (dstorage->ips_available[indice] == 0)
            break;
    }
    return indice;
}

int get_request_info_id(const SERVER_STORAGE *dstorage) {
    return 1;
}