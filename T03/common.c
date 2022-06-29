#include <stdlib.h>
#include <stdio.h>
#include <inttypes.h>
#include <string.h>
#include <arpa/inet.h>

void logExit(char *msg) {
    printf("%s\n", msg);
    exit(1);
}

int addrparse(const char *addrstr, const char *portstr, struct sockaddr_storage *storage) {
    
    if(addrstr == NULL || portstr == NULL || storage == NULL) {
        return -1;
    }

    uint16_t port = (uint16_t)atoi(portstr); // unsigned short
    if(port == 0) {
        return -1;
    }
    port = htons(port); // porta em formato de rede

    struct in_addr inAddr4; // IPv4
    if(inet_pton(AF_INET, addrstr, &inAddr4) == 1) {
        struct sockaddr_in *addr4 = (struct sockaddr_in *)storage;
        addr4->sin_family = AF_INET;
        addr4->sin_port = port;
        addr4->sin_addr = inAddr4;
        return 0;
    }

    struct in_addr inAddr6; // IPv6
    if(inet_pton(AF_INET, addrstr, &inAddr6) == 1) {
        struct sockaddr_in6 *addr6 = (struct sockaddr_in6 *)storage;
        addr6->sin6_family = AF_INET6;
        addr6->sin6_port = port;
        //addr6->sin6_addr = inAddr6;
        memcpy(&addr6->sin6_addr, &inAddr6, sizeof(inAddr6));
        return 0;
    }

    return -1;

}

void addrtostr(const struct sockaddr *addr, char *str, size_t strsize) {

    int version;
    char addrstr[INET6_ADDRSTRLEN+1] = "";
    uint16_t port;

    if(addr->sa_family == AF_INET) { // IPv4
        version = 4;
        if(inet_ntop(AF_INET, &((struct sockaddr_in *)addr)->sin_addr, addrstr, INET_ADDRSTRLEN) == NULL) {
            //strcpy(addrstr, "???");
            logExit("inet_ntop");
        }
        port = ntohs(((struct sockaddr_in *)addr)->sin_port);
    } else if(addr->sa_family == AF_INET6) { // IPv6
        version = 6;
        if(inet_ntop(AF_INET6, &((struct sockaddr_in6 *)addr)->sin6_addr, addrstr, INET6_ADDRSTRLEN) == NULL) {
            //strcpy(addrstr, "???");
            logExit("inet_ntop");
        }
    } else {
        //strncpy(str, "Unknown", strsize);
        logExit("Unknown address family");
    }

    if(str) {
        snprintf(str, strsize, "iPv%d : %s : %hu", version, addrstr, port);
    }
    
}

int server_sockaddr_init(const char *proto, const char *portstr, struct sockaddr_storage *storage) {
    if(proto == NULL || portstr == NULL || storage == NULL) {
        return -1;
    }

    uint16_t port = (uint16_t)atoi(portstr); // unsigned short
    if(port == 0) {
        return -1;
    }
    port = htons(port); // porta em formato de rede

    memset(storage, 0, sizeof(*storage));

    if(strcmp(proto, "v4"/*tcp*/) == 0) {
        ((struct sockaddr_in *)storage)->sin_family = AF_INET;
        ((struct sockaddr_in *)storage)->sin_addr.s_addr = INADDR_ANY;
        ((struct sockaddr_in *)storage)->sin_port = port;
        return 0;
    } else if(strcmp(proto, "v6"/*udp*/) == 0) {
        ((struct sockaddr_in *)storage)->sin_family = AF_INET;
        ((struct sockaddr_in *)storage)->sin_addr.s_addr = INADDR_ANY;
        ((struct sockaddr_in *)storage)->sin_port = port;
        return 0;
    } else {
        return -1;
    }

    


}