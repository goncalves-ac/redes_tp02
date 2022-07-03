//
// Created by thimorais
//
#include "common.h"

#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <arpa/inet.h>
#include <pthread.h>
#include <unistd.h>


void logExit(const char *msg) {
    perror(msg);
    exit(EXIT_FAILURE);
}

int addrparse(const char *addrstr, const char *portstr,
              struct sockaddr_storage *storage) {
    if (addrstr == NULL || portstr == NULL) {
        return -1;
    }

    uint16_t port = (uint16_t) atoi(portstr);
    if (port == 0) {
        return -1;
    }
    port = htons(port);

    struct in_addr inaddr4;
    if (inet_pton(AF_INET, addrstr, &inaddr4)) {
        struct sockaddr_in *addr4 = (struct sockaddr_in *) storage;
        addr4->sin_family = AF_INET;
        addr4->sin_port = port;
        addr4->sin_addr = inaddr4;
        return 0;
    }

    struct in6_addr inaddr6;
    if (inet_pton(AF_INET6, addrstr, &inaddr6)) {
        struct sockaddr_in6 *addr6 = (struct sockaddr_in6 *) storage;
        addr6->sin6_family = AF_INET6;
        addr6->sin6_port = port;
        memcpy(&(addr6->sin6_addr), &inaddr6, sizeof(inaddr6));
        return 0;
    }

    return -1;
}

void addrtostr(const struct sockaddr *addr, char *str, size_t strsize) {
    int version;
    char addrstr[INET6_ADDRSTRLEN + 1] = ""; // +1 for null byte
    uint16_t port;

    if (addr->sa_family == AF_INET) {
        version = 4;
        struct sockaddr_in *addr4 = (struct sockaddr_in *) addr;
        if (!inet_ntop(AF_INET, &(addr4->sin_addr), addrstr,
                       INET6_ADDRSTRLEN + 1)) {
            logExit("ntop");
        }
        port = ntohs(addr4->sin_port);
    } else if (addr->sa_family == AF_INET6) {
        version = 6;
        struct sockaddr_in6 *addr6 = (struct sockaddr_in6 *) addr;
        if (!inet_ntop(AF_INET6, &(addr6->sin6_addr), addrstr,
                       INET6_ADDRSTRLEN + 1)) {
            logExit("ntop");
        }
        port = ntohs(addr6->sin6_port);
    } else {
        logExit("unknown protocol family.");
    }
    if (str) {
        snprintf(str, strsize, "IPv%d %s %hu", version, addrstr, port);
    }
}

int server_sockaddr_init(const char *proto, const char *portstr, struct sockaddr_storage *storage) {
    uint16_t port = (uint16_t) atoi(portstr);
    if (port == 0) {
        return -1;
    }
    port = htons(port);

    memset(storage, 0, sizeof(*storage));
    if (0 == strcmp(proto, "v4")) {
        struct sockaddr_in *addr4 = (struct sockaddr_in *) storage;
        addr4->sin_family = AF_INET;
        addr4->sin_addr.s_addr = INADDR_ANY;
        addr4->sin_port = port;
        return 0;
    } else if (0 == strcmp(proto, "v6")) {
        struct sockaddr_in6 *addr6 = (struct sockaddr_in6 *) storage;
        addr6->sin6_family = AF_INET6;
        addr6->sin6_addr = in6addr_any;
        addr6->sin6_port = port;
        return 0;
    } else {
        return -1;
    }
}

void clientUsage(int argc, char **argv) {
    printf("usage: %s <server IP> <server port>\n", argv[0]);
    printf("example: %s 127.0.0.1 51511\n", argv[0]);
    exit(EXIT_FAILURE);
}

void serverUsage(int argc, char **argv) {
    printf("usage: %s <server port>\n", argv[0]);
    printf("example: %s 51511\n", argv[0]);
    exit(EXIT_FAILURE);
}

char *recuperarIdMensagem(char *string) {
    return strtok(string, " ");
}

int recuperarIdUltimaMensagem(char *string) {
    char *token = strtok(string, " ");
    int id = -1;
    while (token != NULL) {
        if (token != NULL) {
            id = atoi(token);
        }
        token = strtok(NULL, " ");
    }
    return id;
}

void printDataValue(char *string) {
    char *token = strtok(string, " ");
    char id[20];
    while (token != NULL) {
        if (token != NULL) {
            sprintf(id, "%s", token);
        }
        token = strtok(NULL, " ");
    }
    printf("%s\n", id);
}

int recuperarIdDestino(char *string) {
    char *token = strtok(string, " ");
    int id = -1;
    int count = 1;
    while (token != NULL) {
        if (token != NULL && count == 3) {
            id = atoi(token);
        }
        token = strtok(NULL, " ");
        count++;
    }
    return id;
}

int recuperarIdEquipamentoDestino(char *string) {
    char *token = strtok(string, " ");
    int id = -1;
    int count = 1;
    while (token != NULL) {
        if (token != NULL && count == 2) {
            id = atoi(token);
        }
        token = strtok(NULL, " ");
        count++;
    }
    return id;
}

float geradorLeituraAleatoria(int min, int max) {
    float scale = rand() / (float) RAND_MAX;
    return min + scale * (max - min);
}

void broadcast(char *string, int unique_id, struct dadosEquipamento *equipment, pthread_mutex_t lock) {
    pthread_mutex_lock(&lock);
    int i;
    for (i = 0; i < MAXEQUIPAMENTO; ++i) {
        if (equipment[i].equipment_used != -1) {
            if (equipment[i].eq_sock != unique_id) {
                char aux[100];
                sprintf(aux, "%s", string);
                memccpy(string, aux, '\0', 100);
                int id = equipment[i].eq_sock;
                size_t count = write(id, string, strlen(string) + 1);

                if (count < 0) {
                    printf("ERROR: write to descriptor failed\n");
                }
            }
        }
    }
    pthread_mutex_unlock(&lock);
}

void *client_thread(void *data) {
    struct client_data *cdata = (struct client_data *) data;
    struct sockaddr *caddr = (struct sockaddr *) (&cdata->storage);

    char caddrstr[BUFSIZE];
    addrtostr(caddr, caddrstr, BUFSIZE);

    char buf[BUFSIZE];
    char auxBuf[BUFSIZE];
    char payloadBuf[BUFSIZE];
    memset(buf, 0, BUFSIZE);
    size_t count = recv(cdata->csock, buf, BUFSIZE - 1, 0);

    bzero(auxBuf, 256);
    bzero(payloadBuf, 256);
    sprintf(auxBuf, "%s", buf);
    sprintf(payloadBuf, "%s", buf);

    if (strcmp(recuperarIdMensagem(buf), "01") == 0) {
        if (countEquipment >= MAXEQUIPAMENTO) {
            count = send(cdata->csock, "07 15 04", strlen("07 15 04") + 1, 0);
        } else {
            int i;
            for (i = 0; i < MAXEQUIPAMENTO; i++) {
                if (vetorStructEquipamentos[i].equipment_used == -1) {
                    vetorStructEquipamentos[i].equipment_used = 1;
                    char id[10];
                    sprintf(id, "%d", i);
                    memccpy(memccpy(buf, "03 ", '\0', 20) - 1, id, '\0', 20);
                    if (i < 10) {
                        printf("Equipment 0%d added\n", i + 1);
                    } else {
                        printf("Equipment %d added\n", i + 1);
                    }

                    vetorStructEquipamentos[i].eq_sock = cdata->csock;
                    count = send(cdata->csock, buf, strlen(buf) + 1, 0);
                    if (count != strlen(buf) + 1) {
                        logExit("send");
                    }
                    countEquipment++;
                    break;
                }
            }
        }
    } else if (strcmp(recuperarIdMensagem(buf), "02") == 0) {

        countEquipment--;
        int equipment_id = recuperarIdUltimaMensagem(auxBuf);
        sprintf(buf, "08 %d 01", equipment_id);
        if (equipment_id < 9) {
            printf("Equipment 0%d removed\n", equipment_id + 1);
        } else {
            printf("Equipment %d removed\n", equipment_id + 1);
        }
        vetorStructEquipamentos[equipment_id].equipment_used = -1;
        vetorStructEquipamentos[equipment_id].eq_sock = -1;
        broadcast(buf, cdata->csock, vetorStructEquipamentos, lock);
        count = send(cdata->csock, buf, strlen(buf) + 1, 0);
        if (count != strlen(buf) + 1) {
            logExit("send");
        }

    } else if (strcmp(recuperarIdMensagem(buf), "05") == 0) {
        int auxSource = recuperarIdEquipamentoDestino(auxBuf);
        int auxTarget = recuperarIdUltimaMensagem(payloadBuf);
        if ((auxSource - 1) >= MAXEQUIPAMENTO || vetorStructEquipamentos[auxSource - 1].equipment_used == -1) {
            count = send(cdata->csock, "07 15 02", strlen("07 15 02") + 1, 0);
            if (auxSource < 10) {
                printf("Equipment 0%d not found\n", auxSource);
            } else {
                printf("Equipment %d not found\n", auxSource);
            }
        } else if ((auxTarget - 1) >= MAXEQUIPAMENTO || vetorStructEquipamentos[auxTarget - 1].equipment_used == -1) {
            count = send(cdata->csock, "07 15 03", strlen("07 15 03") + 1, 0);
            if (auxTarget < 10) {
                printf("Equipment 0%d not found\n", auxTarget);
            } else {
                printf("Equipment %d not found\n", auxTarget);
            }
        } else {
            char aux[20];
            sprintf(aux, "06 %d %d %.2f", auxSource, auxTarget, geradorLeituraAleatoria(0, 10));
            memccpy(buf, aux, '\0', 20);
            count = send(cdata->csock, buf, strlen(buf) + 1, 0);
            if (count != strlen(buf) + 1) {
                logExit("send");
            }
        }
    } else if (strcmp(recuperarIdMensagem(buf), "09") == 0) {
        int equipment_id = recuperarIdUltimaMensagem(auxBuf);
        bzero(buf, 256);
        int i;
        for (i = 0; i < MAXEQUIPAMENTO; i++) {
            if (i != equipment_id && vetorStructEquipamentos[i].equipment_used == 1) {
                char id[10];
                if (i < 10) {
                    if (strlen(buf) <= 0) {
                        sprintf(id, "04 0%d", i + 1);
                    } else {
                        sprintf(id, " 0%d", i + 1);
                    }
                } else {
                    if (strlen(buf) <= 0) {
                        sprintf(id, "04 %d", i + 1);
                    } else {
                        sprintf(id, " %d", i + 1);
                    }
                }

                memccpy(memccpy(buf, buf, '\0', 20) - 1, id, '\0', 20);
            }
        }
        if (strlen(buf) <= 0) {
            memccpy(memccpy(buf, buf, '\0', 20) - 1, "04", '\0', 20);
        }

        count = send(cdata->csock, buf, strlen(buf) + 1, 0);
        if (count != strlen(buf) + 1) {
            logExit("send");
        }
    } else {
        memccpy(buf, "I got your message", '\0', strlen("I got your message") + 1);
        count = send(cdata->csock, buf, strlen(buf) + 1, 0);
        if (count != strlen(buf) + 1) {
            logExit("send");
        }
    }
    close(cdata->csock);
    pthread_exit(EXIT_SUCCESS);
}