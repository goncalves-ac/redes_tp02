//
// Created by thimorais
//
#include "common.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <pthread.h>

int main(int argc, char *argv[]) {
    if (argc < 2) {
        serverUsage(argc, argv);
    }

    int i;
    for (i = 0; i < CLIENTS; i++) {
        equipment[i].equipment_used = -1;
        equipment[i].eq_sock = -1;
    }

    if (pthread_mutex_init(&lock, NULL) != 0) {
        printf("mutex init failed");
        return 1;
    }

    struct sockaddr_storage storage; // Socket address storage
    if (0 != server_sockaddr_init("v6", argv[1], &storage)) {
        serverUsage(argc, argv);
    }

    int s; // Socket descriptor
    s = socket(storage.ss_family, SOCK_STREAM, 0);
    if (s == -1) {
        logExit("socket");
    }

    int enable = 1; // Enable option for setsockopt
    if (0 != setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int))) {
        logExit("setsockopt");
    }

    struct sockaddr *addr = (struct sockaddr *) (&storage); // Socket address
    if (0 != bind(s, addr, sizeof(storage))) {
        logExit("bind");
    }

    if (0 != listen(s, 10)) { // Listen on socket
        logExit("listen");
    }

    char addrstr[BUFSIZE];
    addrtostr(addr, addrstr, BUFSIZE); // Convert address to string

    for (;;) {
        struct sockaddr_storage cstorage;  // Client socket address storage
        struct sockaddr *caddr = (struct sockaddr *) (&cstorage);
        socklen_t caddrlen = sizeof(cstorage);

        int csock = accept(s, caddr, &caddrlen);
        if (csock == -1) {
            logExit("accept");
        }

        struct client_data *cdata = malloc(sizeof(*cdata));
        if (!cdata) {
            logExit("malloc");
        }

        cdata->csock = csock; // Save socket descriptor
        memcpy(&(cdata->storage), &cstorage, sizeof(cstorage)); // Save socket address

        pthread_t tid; // Thread descriptor
        pthread_create(&tid, NULL, client_thread, cdata); // Create thread
    }
//    exit(EXIT_SUCCESS);
}
