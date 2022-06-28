#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <arpa/inet.h>

void ERROR(const char *msg) {
    perror(msg);
    exit(1);
}

static void
broadcast(const char *mess)
{
    struct sockaddr_in s;

    if(broadcastSock < 0)
        return;

    memset(&s, '\0', sizeof(struct sockaddr_in));
    s.sin_family = AF_INET;
    s.sin_port = (in_port_t)htons(tcpSocket ? tcpSocket : 3310);
    s.sin_addr.s_addr = htonl(INADDR_BROADCAST);

    cli_dbgmsg("broadcast %s to %d\n", mess, broadcastSock);
    if(sendto(broadcastSock, mess, strlen(mess), 0, (struct sockaddr *)&s, sizeof(struct sockaddr_in)) < 0)
        perror("sendto");
}
