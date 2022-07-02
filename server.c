#include "common.h"

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/socket.h>
#include <sys/types.h>

struct equipment_data equipment[CLIENTS];

pthread_mutex_t lock;
int countEquipment = 0;

void usage(int argc, char *argv[]) { // inform how to use the program
    printf("usage: %s <server port>\n", argv[0]);
    printf("example: %s 51511\n", argv[0]);
    exit(EXIT_FAILURE);
}

void * client_thread(void *data) {
    int equipment_id = -1;

    struct client_data *cdata = (struct client_data *)data;
    struct sockaddr *caddr = (struct sockaddr *)(&cdata->storage);

    char caddrstr[BUFSIZE];
    addrtostr(caddr, caddrstr, BUFSIZE);

    char buf[BUFSIZE];
    char auxBuf[BUFSIZE];
    char payloadBuf[BUFSIZE];
    memset(buf, 0, BUFSIZE);
    size_t count = recv(cdata->csock, buf, BUFSIZE - 1, 0);

    // clear auxiliary buffer
    bzero(auxBuf,256); 
    bzero(payloadBuf,256);
    sprintf(auxBuf,"%s",buf);
    sprintf(payloadBuf,"%s",buf);

    // According to the message sent by the equipment in internal encoding, the server decodes and forwards the requested response.
    // Equipment is add in server's database.
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
    } else if (strcmp(getMessageId(buf), "02") == 0) // Equipment requests server output.
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
        send_message_to_all(buf, cdata->csock, equipment, lock);
        count = send(cdata->csock, buf, strlen(buf) + 1, 0);
        if (count != strlen(buf) + 1) {
            logExit("send");
        }
        
    } else if (strcmp(getMessageId(buf), "05") == 0) // Equipment requests information about some equipment in database and server response.
    {
        int auxSource = getEquipmentIdWithPayload(auxBuf);
        int auxTarget = getLastMessageId(payloadBuf);
        if ((auxSource - 1) >= CLIENTS || equipment[auxSource -1].equipment_used == -1)
        {
            count = send(cdata->csock, "07 15 02", strlen("07 15 02") + 1, 0); // Equipment is not in database.
            if(auxSource < 10) {
                printf("Equipment 0%d not found\n", auxSource);
            } else {
                printf("Equipment %d not found\n", auxSource);
            }
        } else if ((auxTarget - 1) >= CLIENTS || equipment[auxTarget -1].equipment_used == -1)
        {
            count = send(cdata->csock, "07 15 03", strlen("07 15 03") + 1, 0); // Equipment is not in database.
            if(auxTarget < 10) {
                printf("Equipment 0%d not found\n", auxTarget);
            } else {
                printf("Equipment %d not found\n", auxTarget);
            }
        } else {
            char aux[20];
            sprintf(aux, "06 %d %d %.2f", auxSource, auxTarget, rand_float(0, 10));
            memccpy(buf, aux, '\0', 20);
            count = send(cdata->csock, buf, strlen(buf) + 1, 0); // Equipment is in database.
            if (count != strlen(buf) + 1) {
                logExit("send");
            }
        }
    } else if (strcmp(getMessageId(buf), "09") == 0) // code created to return the equipment list to the requesting equipment
    {
        int equipment_id = getLastMessageId(auxBuf); //equipment_id is the equipment that requested the list
        bzero(buf,256);
        for (int i = 0; i < CLIENTS; i++)
        {
            if(i != equipment_id && equipment[i].equipment_used == 1) //if the equipment is not the one that requested the list and is in the database
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
                
                memccpy(memccpy(buf, buf, '\0', 20) - 1, id, '\0', 20); //add the equipment to the list
            }
        }
        if (strlen(buf) <= 0)
        {
            memccpy(memccpy(buf, buf, '\0', 20) - 1, "04", '\0', 20); //if the list is empty, add the message "04" to the list
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

int main(int argc, char *argv[]) {
    if (argc < 2) {
        usage(argc, argv);
    }

    // Populating equipment array
    for (int i = 0; i < CLIENTS; i++)
    {
        equipment[i].equipment_used = -1;
        equipment[i].eq_sock = -1;
    }
    
    if (pthread_mutex_init(&lock, NULL) != 0)
    {
        printf("mutex init failed");
        return 1;
    }
    
    struct sockaddr_storage storage; // Socket address storage
    if (0 != server_sockaddr_init("v6", argv[1], &storage)) {
        usage(argc, argv);
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

    struct sockaddr *addr = (struct sockaddr *)(&storage); // Socket address
    if (0 != bind(s, addr, sizeof(storage))) {
        logExit("bind");
    }

    if (0 != listen(s, 10)) { // Listen on socket
        logExit("listen");
    }

    char addrstr[BUFSIZE]; 
    addrtostr(addr, addrstr, BUFSIZE); // Convert address to string
    //printf("bound to %s, waiting connections\n", addrstr);

    while (1) {
        struct sockaddr_storage cstorage;  // Client socket address storage
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

        cdata->csock = csock; // Save socket descriptor
        memcpy(&(cdata->storage), &cstorage, sizeof(cstorage)); // Save socket address

        pthread_t tid; // Thread descriptor
        pthread_create(&tid, NULL, client_thread, cdata); // Create thread
    }

    exit(EXIT_SUCCESS);
}
