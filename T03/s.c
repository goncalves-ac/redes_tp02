#include "common.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#define BUFSIZE 1024

void usage(int argc, char **argv) {
    printf("Usage: %s v4|v6 <server PORT>\n", argv[0]);
    printf("Exemplo: %s v4 51511\n", argv[0]);
    exit(EXIT_FAILURE);
}

int main(int argc, char **argv) {
    if(argc < 3) {
        usage(argc, argv);
    }

    struct sockaddr_storage storage;
    if (0 != server_sockaddr_init(argv[1], argv[2], &storage)) {
        usage(argc, argv);
    }

    int s;
    s = socket(storage.ss_family, SOCK_STREAM, 0);
    if(s == -1) {
        logExit("socket");
    }

    struct sockaddr *addr = (struct sockaddr *)&storage; // cast
    if(bind(s, addr, sizeof(storage)) != 0) {
        logExit("bind");
    }

    if(listen(s, 10) != 0) { // 10 é o número máximo de conexões pendentes
        logExit("listen");
    }

    char addrstr[BUFSIZE];
    addrtostr(addr, addrstr, BUFSIZE);
    printf("Listening on %s\n - waiting connection", addrstr);

    while(1) {
        struct sockaddr_storage client_storage;
        struct sockaddr *client_addr = (struct sockaddr *)&client_storage; // cast
        socklen_t client_len = sizeof(client_storage);
        
        int client_sockt = accept(s, client_addr, &client_len);
        if(client_sockt == -1) {
            logExit("accept");
        }

        char client_addrstr[BUFSIZE];
        addrtostr(client_addr, client_addrstr, BUFSIZE);
        printf("[log] Connection from %s\n", client_addrstr);

        char buf[BUFSIZE];
        memset(buf, 0, BUFSIZE); // zera o buffer
        size_t count = recv(client_sockt, buf, BUFSIZE-1, 0);
        printf("[log] Received %ld bytes\n", count);
        printf("[msg] %s : %s\n", client_addrstr, buf);

        sprintf(buf,"[server] Address: %.1000s\n", client_addrstr);
        count = send(client_sockt, buf, strlen(buf)+1, 0);
         if(count != strlen(buf)+1) {
            logExit("send");
        }

        close(client_sockt);

    }

    exit(EXIT_SUCCESS);
}