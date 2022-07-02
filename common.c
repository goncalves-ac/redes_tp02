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
    if (addrstr == NULL || portstr == NULL) { // verifica se os argumentos são nulos
        return -1;
    }

    uint16_t port = (uint16_t) atoi(portstr); // unsigned short
    if (port == 0) {
        return -1;
    }
    port = htons(port); // host to network short

    struct in_addr inaddr4; // 32-bit IP address
    if (inet_pton(AF_INET, addrstr, &inaddr4)) {
        struct sockaddr_in *addr4 = (struct sockaddr_in *) storage;
        addr4->sin_family = AF_INET;
        addr4->sin_port = port;
        addr4->sin_addr = inaddr4;
        return 0;
    }

    struct in6_addr inaddr6; // 128-bit IPv6 address
    if (inet_pton(AF_INET6, addrstr, &inaddr6)) {
        struct sockaddr_in6 *addr6 = (struct sockaddr_in6 *) storage;
        addr6->sin6_family = AF_INET6;
        addr6->sin6_port = port;
        // addr6->sin6_addr = inaddr6
        memcpy(&(addr6->sin6_addr), &inaddr6, sizeof(inaddr6));
        return 0;
    }

    return -1;
}

void addrtostr(const struct sockaddr *addr, char *str, size_t strsize) {
    int version; // IPv4 or IPv6
    char addrstr[INET6_ADDRSTRLEN + 1] = ""; // +1 for null byte
    uint16_t port; // network byte order

    if (addr->sa_family == AF_INET) {
        version = 4;
        struct sockaddr_in *addr4 = (struct sockaddr_in *) addr; // IPv4 address
        if (!inet_ntop(AF_INET, &(addr4->sin_addr), addrstr,
                       INET6_ADDRSTRLEN + 1)) { // IPv4 address to string
            logExit("ntop");
        }
        port = ntohs(addr4->sin_port); // network to host short
    } else if (addr->sa_family == AF_INET6) {
        version = 6;
        struct sockaddr_in6 *addr6 = (struct sockaddr_in6 *) addr; // IPv6 address
        if (!inet_ntop(AF_INET6, &(addr6->sin6_addr), addrstr,
                       INET6_ADDRSTRLEN + 1)) { // IPv6 address to string
            logExit("ntop");
        }
        port = ntohs(addr6->sin6_port); // network to host short
    } else {
        logExit("unknown protocol family.");
    }
    if (str) {
        snprintf(str, strsize, "IPv%d %s %hu", version, addrstr, port); // IPv4 or IPv6 address and port
    }
}

int server_sockaddr_init(const char *proto, const char *portstr,
                         struct sockaddr_storage *storage) {
    uint16_t port = (uint16_t) atoi(portstr); // unsigned short
    if (port == 0) {
        return -1;
    }
    port = htons(port); // host to network short

    memset(storage, 0, sizeof(*storage));
    if (0 == strcmp(proto, "v4")) {
        struct sockaddr_in *addr4 = (struct sockaddr_in *) storage;
        addr4->sin_family = AF_INET; // IPv4
        addr4->sin_addr.s_addr = INADDR_ANY; // any interface
        addr4->sin_port = port; // port
        return 0;
    } else if (0 == strcmp(proto, "v6")) {
        struct sockaddr_in6 *addr6 = (struct sockaddr_in6 *) storage;
        addr6->sin6_family = AF_INET6; // IPv6
        addr6->sin6_addr = in6addr_any; // any interface
        addr6->sin6_port = port;// port
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
    // return the message id from to print the message
    return strtok(string, " ");
}

int getLastMessageId(char *string) {
    // return the last message id from to print the message
    char *token = strtok(string, " "); // get the first token, this case id of the message
    int id = -1;
    while (token != NULL) {
        if (token != NULL) {
            id = atoi(token); // convert the token to int, to know the id message or equipment
        }
        token = strtok(NULL, " "); // get the next token
    }

    return id;
}

void printDataValue(char *string) {
    // return the data value from to print the message when need to information from equipment
    char *token = strtok(string, " "); // get the first token, this case id of the message
    char id[20];
    while (token != NULL) {
        if (token != NULL) {
            sprintf(id, "%s", token);
        }
        token = strtok(NULL, " "); // get the next token
    }

    printf("%s\n", id);
}

int getTargetId(char *string) {
    // returns the device id on which to return the requested information 
    char *token = strtok(string, " "); // get the first token
    int id = -1;
    int count = 1;
    while (token != NULL) {
        if (token != NULL && count ==
                             3) { // this this case de position of the device id is 3, always - the according to the issue specification.
            id = atoi(token);
        }
        token = strtok(NULL, " ");
        count++;
    }

    return id;
}

int getEquipmentIdWithPayload(char *string) {
    // return the id of the equipment where you need to send the payload
    char *token = strtok(string, " ");
    int id = -1;
    int count = 1;
    while (token != NULL) {
        if (token != NULL && count ==
                             2) { // this this case de position of the device id is 2, always - the according to the issue specification.
            id = atoi(token);
        }
        token = strtok(NULL, " ");
        count++;
    }

    return id;
}

float geradorLeituraAleatoria(int min, int max) {
    // Generate random numbers
    float scale = rand() / (float) RAND_MAX; // [0, 1.0]
    return min + scale * (max - min); // [min, max]
}

void broadcast(char *string, int unique_id, struct equipment_data *equipment, pthread_mutex_t lock) {
    pthread_mutex_lock(&lock);
    // Send the message to all clients except the sender
    int i;
    for (i = 0; i < CLIENTS; ++i) {
        if (equipment[i].equipment_used != -1) {
            if (equipment[i].eq_sock != unique_id) {
                char aux[100];
                sprintf(aux, "%s", string); // copy the string to a new one, to avoid to modify the original one
                memccpy(string, aux, '\0', 100); // copy the string to the assistant string, to avoid the memory leak
                int id = equipment[i].eq_sock; // get the id of the device to which the message should be forwarded
                //size_t count = send(equipment[i].eq_sock, string, strlen(string) + 1, 0);
                size_t count = write(id, string, strlen(string) + 1); // send the message to the equipment

                if (count < 0) {
                    printf("ERROR: write to descriptor failed\n");
                }
            }
        }
    }

    pthread_mutex_unlock(&lock);
}

void *client_thread(void *data) {
//    int equipment_id = -1;

    struct client_data *cdata = (struct client_data *) data;
    struct sockaddr *caddr = (struct sockaddr *) (&cdata->storage);

    char caddrstr[BUFSIZE];
    addrtostr(caddr, caddrstr, BUFSIZE);

    char buf[BUFSIZE];
    char auxBuf[BUFSIZE];
    char payloadBuf[BUFSIZE];
    memset(buf, 0, BUFSIZE);
    size_t count = recv(cdata->csock, buf, BUFSIZE - 1, 0);

    // clear auxiliary buffer
    bzero(auxBuf, 256);
    bzero(payloadBuf, 256);
    sprintf(auxBuf, "%s", buf);
    sprintf(payloadBuf, "%s", buf);

    // According to the message sent by the equipment in internal encoding, the server decodes and forwards the requested response.
    // Equipment is add in server's database.
    if (strcmp(recuperarIdMensagem(buf), "01") == 0) {
        if (countEquipment >= CLIENTS) {
            count = send(cdata->csock, "07 15 04", strlen("07 15 04") + 1, 0);
        } else {
            int i;
            for (i = 0; i < CLIENTS; i++) {
                if (equipment[i].equipment_used == -1) {
                    equipment[i].equipment_used = 1;
                    char id[10];
                    sprintf(id, "%d", i);
                    memccpy(memccpy(buf, "03 ", '\0', 20) - 1, id, '\0', 20);
                    if (i < 10) {
                        printf("Equipment 0%d added\n", i + 1);
                    } else {
                        printf("Equipment %d added\n", i + 1);
                    }

                    equipment[i].eq_sock = cdata->csock;
                    count = send(cdata->csock, buf, strlen(buf) + 1, 0);
                    if (count != strlen(buf) + 1) {
                        logExit("send");
                    }
                    countEquipment++;
                    break;
                }
            }
        }
    } else if (strcmp(recuperarIdMensagem(buf), "02") == 0) // Equipment requests server output.
    {

        countEquipment--;
        int equipment_id = getLastMessageId(auxBuf);
        sprintf(buf, "08 %d 01", equipment_id);
        if (equipment_id < 9) {
            printf("Equipment 0%d removed\n", equipment_id + 1);
        } else {
            printf("Equipment %d removed\n", equipment_id + 1);
        }
        equipment[equipment_id].equipment_used = -1;
        equipment[equipment_id].eq_sock = -1;
        broadcast(buf, cdata->csock, equipment, lock);
        count = send(cdata->csock, buf, strlen(buf) + 1, 0);
        if (count != strlen(buf) + 1) {
            logExit("send");
        }

    } else if (strcmp(recuperarIdMensagem(buf), "05") ==
               0) // Equipment requests information about some equipment in database and server response.
    {
        int auxSource = getEquipmentIdWithPayload(auxBuf);
        int auxTarget = getLastMessageId(payloadBuf);
        if ((auxSource - 1) >= CLIENTS || equipment[auxSource - 1].equipment_used == -1) {
            count = send(cdata->csock, "07 15 02", strlen("07 15 02") + 1, 0); // Equipment is not in database.
            if (auxSource < 10) {
                printf("Equipment 0%d not found\n", auxSource);
            } else {
                printf("Equipment %d not found\n", auxSource);
            }
        } else if ((auxTarget - 1) >= CLIENTS || equipment[auxTarget - 1].equipment_used == -1) {
            count = send(cdata->csock, "07 15 03", strlen("07 15 03") + 1, 0); // Equipment is not in database.
            if (auxTarget < 10) {
                printf("Equipment 0%d not found\n", auxTarget);
            } else {
                printf("Equipment %d not found\n", auxTarget);
            }
        } else {
            char aux[20];
            sprintf(aux, "06 %d %d %.2f", auxSource, auxTarget, geradorLeituraAleatoria(0, 10));
            memccpy(buf, aux, '\0', 20);
            count = send(cdata->csock, buf, strlen(buf) + 1, 0); // Equipment is in database.
            if (count != strlen(buf) + 1) {
                logExit("send");
            }
        }
    } else if (strcmp(recuperarIdMensagem(buf), "09") ==
               0) // code created to return the equipment list to the requesting equipment
    {
        int equipment_id = getLastMessageId(auxBuf); //equipment_id is the equipment that requested the list
        bzero(buf, 256);
        int i;
        for (i = 0; i < CLIENTS; i++) {
            if (i != equipment_id && equipment[i].equipment_used ==
                                     1) //if the equipment is not the one that requested the list and is in the database
            {
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

                memccpy(memccpy(buf, buf, '\0', 20) - 1, id, '\0', 20); //add the equipment to the list
            }
        }
        if (strlen(buf) <= 0) {
            memccpy(memccpy(buf, buf, '\0', 20) - 1, "04", '\0',
                    20); //if the list is empty, add the message "04" to the list
        }

        count = send(cdata->csock, buf, strlen(buf) + 1, 0); //send the list to the equipment that requested it
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

    close(cdata->csock); // close connection

    pthread_exit(EXIT_SUCCESS); // exit thread
}