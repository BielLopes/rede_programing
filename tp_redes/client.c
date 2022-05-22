#include "common.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

void usage(int argc, char **argv)
{
    if (argc >= 4)
    {
        printf("usage: ./%s <server IP> <server port>\n", argv[0]);
        printf("exemple: ./%s 127.0.0.1 51511\n", argv[0]);
    }
    exit(EXIT_FAILURE);
}

int main(int argc, char **argv)
{
    if (argc < 3)
        usage(argc, argv);

    struct sockaddr_storage storage;
    if (0 != addrparse(argv[1], argv[2], &storage))
        usage(argc, argv);

    int s;
    s = socket(storage.ss_family, SOCK_STREAM, 0);
    if (s == -1)
        logexit("socket");

    struct sockaddr *addr = (struct sockaddr *)(&storage);
    if (0 != connect(s, addr, sizeof(storage)))
        logexit("connect");

    char buf[BUFSZ];
    while (1)
    {
        memset(buf, 0, BUFSZ); // Clean Buffer

        // Read from keyboard the next command to server
        printf("> ");
        fgets(buf, BUFSZ - 1, stdin);
        size_t count = send(s, buf, strlen(buf), 0);

        if (count != strlen(buf))
            logexit("send");

        memset(buf, 0, BUFSZ);
        count = recv(s, buf, BUFSZ, 0);

        printf("< %s\n", buf);
    }

    close(s);

    exit(EXIT_SUCCESS);
}