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
    printf("Usage: %s <server IP> <server PORT>\n", argv[0]);
    printf("Exemplo: %s 127.0.0.1 51511\n", argv[0]);
    exit(EXIT_FAILURE);
}

int main(int argc, char **argv) {
    if(argc < 3) {
        usage(argc, argv);
    }

    struct sockaddr_storage storage;
    if (0 != addrparse(argv[1], argv[2], &storage)) {
        usage(argc, argv);
    }

    int s;
    s = socket(storage.ss_family, SOCK_STREAM, 0);
    if(s == -1) {
        logExit("socket");
    }

    struct sockaddr *addr = (struct sockaddr *)(&storage); // cast
    if(connect(s, addr, sizeof(storage)) == -1) {
        logExit("connect");
    }

    char addrstr[BUFSIZE];
    addrtostr(addr, addrstr, BUFSIZE);

    printf("Connected to %s\n", addrstr);

    //Manda a msg
    char buf[BUFSIZE];
    memset(buf, 0, BUFSIZE); // limpa o buffer
    printf("Enter a message: ");
    fgets(buf, BUFSIZE-1, stdin);
    size_t count = send(s, buf, strlen(buf)+1, 0); // O +1 é para o \n
    // send retorna o número de bytes enviados

    if(count != strlen(buf)+1) {
        logExit("send");
    }

    //Recebe a msg
    memset(buf, 0, BUFSIZE);
    unsigned totalBytes = 0; // total de bytes recebidos

    while (1) {

        count = recv(s, buf + totalBytes, BUFSIZE - totalBytes, 0);

        if(count == 0) {
            // Close terminated
            logExit("recv");
            break;
        }
        totalBytes += count;
        printf("Received: %s\n", buf);

    }

    close(s);

    printf("Received %d bytes.\n", totalBytes);
    puts(buf);

    exit(EXIT_SUCCESS);

}