#include "common.h"

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/socket.h>
#include <sys/types.h>

void usage(int argc, char *argv[]) {
    printf("usage: %s <server port>\n", argv[0]);
    printf("example: %s 51511\n", argv[0]);
    exit(EXIT_FAILURE);
}

struct equipment_data equipment[CLIENTS];
int equipment_id = -1;
int countEquipment = 0;


void connection_close_notice(char *s, int uid)
{
    for (int i = 0; i < CLIENTS; ++i)
    {
        if (equipment[i].csock != uid && equipment[i].csock != -1)
        {
            int count = write(equipment[i].csock, s, strlen(s) + 1);
            if (count != strlen(s) + 1)
            {
                perror("ERROR: write to descriptor failed");
                break;
            }
        }
    }
}


void * client_thread(void *data) {
    struct client_data *cdata = (struct client_data *)data;
    struct sockaddr *caddr = (struct sockaddr *)(&cdata->storage);

    char caddrstr[BUFSIZE];
    addrtostr(caddr, caddrstr, BUFSIZE);

    char buf[BUFSIZE];
    char auxBuf[BUFSIZE];
    char payloadBuf[BUFSIZE];
    memset(buf, 0, BUFSIZE);
    size_t count = recv(cdata->csock, buf, BUFSIZE - 1, 0);

    bzero(auxBuf,256);
    bzero(payloadBuf,256);
    sprintf(auxBuf,"%s",buf);
    sprintf(payloadBuf,"%s",buf);

    if (strcmp(getMessageId(buf), "01") == 0)
    {
        if(countEquipment >= CLIENTS)
        {
            count = send(cdata->csock, "07 15 04", strlen("07 15 04") + 1, 0);
        } else {
            for (int i = 0; i < CLIENTS; i++)
            {
                if(equipment[i].equipment_used == -1)
                {
                    equipment[i].equipment_used = 1;
                    char id[10];
                    sprintf(id, "%d", i);
                    memccpy(memccpy(buf, "03 ", '\0', 20) - 1, id, '\0', 20);
                    if (i < 10) {
                        printf("Equipment 0%d added\n", i + 1);
                    } else {
                        printf("Equipment %d added\n", i + 1);
                    }
                    
                    equipment[i].csock = cdata->csock;
                    count = send(cdata->csock, buf, strlen(buf) + 1, 0);
                    if (count != strlen(buf) + 1) {
                        logExit("send");
                    }
                    countEquipment++;
                    break;
                }
            }
        }
    } else if (strcmp(getMessageId(buf), "02") == 0)
    {
        
        countEquipment--;
        int equipment_id = getLastMessageId(auxBuf);
        sprintf(buf, "08 %d 01", equipment_id);
        if (equipment_id < 10) {
            printf("Equipment 0%d removed\n", equipment_id + 1);
        } else {
            printf("Equipment %d removed\n", equipment_id + 1);
        }
        equipment[equipment_id].equipment_used = -1;
        equipment[equipment_id].csock = -1;
        count = send(cdata->csock, buf, strlen(buf) + 1, 0);
        if (count != strlen(buf) + 1) {
            logExit("send");
        }
        //connection_close_notice(buf, cdata->csock);
    } else if (strcmp(getMessageId(buf), "05") == 0)
    {
        int auxSource = getEquipmentIdWithPayload(auxBuf);
        int auxTarget = getLastMessageId(payloadBuf);
        if (equipment[auxSource -1].equipment_used == -1)
        {
            count = send(cdata->csock, "07 15 02", strlen("07 15 02") + 1, 0);
        } else if (equipment[auxTarget -1].equipment_used == -1)
        {
            count = send(cdata->csock, "07 15 03", strlen("07 15 03") + 1, 0);
        } else {
            char aux[20];
            sprintf(aux, "06 %d %d %.2f", auxSource, auxTarget, rand_float(0, 10));
            memccpy(buf, aux, '\0', 20);
            count = send(cdata->csock, buf, strlen(buf) + 1, 0);
            if (count != strlen(buf) + 1) {
                logExit("send");
            }
        }
        
        

    } else if (strcmp(getMessageId(buf), "09") == 0)
    {
        int equipment_id = getLastMessageId(auxBuf);
        bzero(buf,256);
        for (int i = 0; i < CLIENTS; i++)
        {
            if(i != equipment_id && equipment[i].equipment_used == 1)
            {
                char id[10];
                if (i < 10)
                {
                    if (strlen(buf) <= 0)
                    {
                        sprintf(id, "04 0%d", i+1);
                    } else {
                        sprintf(id, " 0%d", i+1);
                    }
                } else {
                    if (strlen(buf) <= 0)
                    {
                        sprintf(id, "04 %d", i+1);
                    } else {
                        sprintf(id, " %d", i+1);
                    }
                }
                
                memccpy(memccpy(buf, buf, '\0', 20) - 1, id, '\0', 20);
            }
        }
        if (strlen(buf) <= 0)
        {
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

int main(int argc, char *argv[]) {
    if (argc < 2) {
        usage(argc, argv);
    }

    // Populando array de equipamentos
    for (int i = 0; i < CLIENTS; i++)
    {
        equipment[i].equipment_used = -1;
        equipment[i].csock = -1;
    }

    struct sockaddr_storage storage;
    if (0 != server_sockaddr_init("v6", argv[1], &storage)) {
        usage(argc, argv);
    }

    int s;
    s = socket(storage.ss_family, SOCK_STREAM, 0);
    if (s == -1) {
        logExit("socket");
    }

    int enable = 1;
    if (0 != setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int))) {
        logExit("setsockopt");
    }

    struct sockaddr *addr = (struct sockaddr *)(&storage);
    if (0 != bind(s, addr, sizeof(storage))) {
        logExit("bind");
    }

    if (0 != listen(s, 10)) {
        logExit("listen");
    }

    char addrstr[BUFSIZE];
    addrtostr(addr, addrstr, BUFSIZE);
    printf("bound to %s, waiting connections\n", addrstr);

    while (1) {
        struct sockaddr_storage cstorage;
        struct sockaddr *caddr = (struct sockaddr *)(&cstorage);
        socklen_t caddrlen = sizeof(cstorage);

        int csock = accept(s, caddr, &caddrlen);
        if (csock == -1) {
            logExit("accept");
        }

        struct client_data *cdata = malloc(sizeof(*cdata));
        if (!cdata) {
            logExit("malloc");
        }

        cdata->csock = csock;
        memcpy(&(cdata->storage), &cstorage, sizeof(cstorage));

        pthread_t tid;
        pthread_create(&tid, NULL, client_thread, cdata);
    }

    exit(EXIT_SUCCESS);
}
