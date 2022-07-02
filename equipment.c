#include "common.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>

int equipment_client[CLIENTS];

void usage(int argc, char **argv) { // inform how to use the program
    printf("Usage: %s <server IP> <server PORT>\n", argv[0]);
    printf("Exemplo: %s 127.0.0.1 51511\n", argv[0]);
    exit(EXIT_FAILURE);
}

int main(int argc, char **argv) {
    int s;
    struct sockaddr_storage storage;
    struct sockaddr *addr;
    char addrstr[BUFSIZE];
    char buf[BUFSIZE];
    char auxBuf[BUFSIZE];
    char payloadBuf[BUFSIZE];
    char dataBuf[BUFSIZE];
    unsigned totalBytes = 0; // total bytes received
    size_t count;
    int equipment_id = -1;

    if(argc < 3) {
        usage(argc, argv);
    }

    // Populating equipment's id array
    // -1 - don't have equipment
    // 1 - have equipment
    // 0 - is the owner of the id
    for (int i = 0; i < CLIENTS; i++)
    {
        equipment_client[i] = -1;
    }
    
    while (1)
    {
        if (0 != addrparse(argv[1], argv[2], &storage)) {
            usage(argc, argv);
        }

        s = socket(storage.ss_family, SOCK_STREAM, 0);
        if(s == -1) {
            logExit("socket");
        }

        addr = (struct sockaddr *)(&storage); // cast
        if(connect(s, addr, sizeof(storage)) == -1) {
            logExit("connect");
        }

        addrtostr(addr, addrstr, BUFSIZE);
        totalBytes = 0; // total de bytes recebidos
        
        //send the msg
        if (equipment_id == -1){
            memset(buf, 0, BUFSIZE); // clear the buffer
            sprintf(buf, "01");
            count = send(s, buf, strlen(buf)+1, 0); // the +1 is to \n
            // send - return the number of bytes sent
        } else {
            memset(buf, 0, BUFSIZE); // clear the buffer
            //printf(">>> ");
            fgets(buf, BUFSIZE-1, stdin); // read the message     
            sprintf(auxBuf,"%s",buf);
            if (strcmp(buf, "close connection\n") == 0 || strcmp(buf, "close connection") == 0) // request for server output
            {
                memset(buf, 0, BUFSIZE); // clear the buffer
                sprintf(buf, "02 %d", equipment_id);
                count = send(s, buf, strlen(buf)+1, 0);
            } else if (strcmp(buf, "list equipment\n") == 0 || strcmp(buf, "list equipment") == 0) // request the list online in the moment
            {
                memset(buf, 0, BUFSIZE); // clear the buffer
                sprintf(buf, "09 %d", equipment_id);
                count = send(s, buf, strlen(buf)+1, 0); 
            }else if (strstr(buf, "request information from"))
            {
                int target_equipment = getLastMessageId(auxBuf);
                memset(buf, 0, BUFSIZE); // clear the buffer
                sprintf(buf, "05 %d %d", equipment_id, target_equipment);
                count = send(s, buf, strlen(buf)+1, 0); 
            }
        }        

        //Receive the message
        memset(buf, 0, BUFSIZE);
        totalBytes = 0; // total bytes received
        count = recv(s, buf + totalBytes, BUFSIZE - totalBytes, 0); // receive the message
        totalBytes += count;
        bzero(auxBuf,256);
        bzero(payloadBuf,256);
        bzero(dataBuf,256);
        sprintf(auxBuf,"%s",buf);
        sprintf(payloadBuf, "%s", buf);
        sprintf(dataBuf, "%s", buf);
        if (strcmp(getMessageId(buf), "03") == 0) // return from add request to server
        {
            if (equipment_id == -1)
            {
                equipment_id = getLastMessageId(auxBuf);
                equipment_client[equipment_id] = 0;
                if (equipment_id < 10) {
                    printf("New ID: 0%d\n", equipment_id + 1);
                } else {
                    printf("New ID: %d\n", equipment_id + 1);
                }
            } else {
                int aux_equipment_id = getLastMessageId(auxBuf);
                equipment_client[aux_equipment_id] = 1;
                if (aux_equipment_id < 10) {
                    printf("Equipment 0%d added\n", aux_equipment_id + 1);
                } else {
                    printf("Equipment %d added\n", aux_equipment_id + 1);
                }
            }   
        
        } else if (strcmp(getMessageId(buf), "04") == 0) // answer of what equipment is online at the moment
        {
            if (strlen(auxBuf) > 3)
            {
                printf("%s\n", auxBuf + 3);
            }
        }else if (strcmp(getMessageId(buf), "06") == 0) // request the information about some equipment
        {
            int sourceId = getEquipmentIdWithPayload(auxBuf);
            int targetId = getTargetId(payloadBuf);
            if (sourceId - 1 == equipment_id)
            {
                printf("requested information\n");
            } else {
                if (targetId < 10)
                {
                    printf("Value from 0%d: ", targetId);
                } else 
                {            
                    printf("Value from %d: ", targetId);
                }
                printDataValue(dataBuf);
            }
            
        } else if (strcmp(getMessageId(buf), "07") == 0) // message about some error in the request
        {
            //int equipmentId = getEquipmentIdWithPayload(auxBuf);
            int payload = getLastMessageId(payloadBuf);
            
            if (payload == 1)
            {
                printf("Equipment not found\n");
            } else if (payload == 2)
            {
                printf("Source equipment not found\n");
            } else if (payload == 3)
            {
                printf("Target equipment not found\n");
            } else
            {
                printf("Equipment limit exceeded\n");
                bzero(buf,256);
                bzero(auxBuf,256);
                bzero(payloadBuf,256);
                bzero(dataBuf,256);
                break;
            }
        } else if (strcmp(getMessageId(buf), "08") == 0) // server exit successful
        {
            int aux_equipment_id = getEquipmentIdWithPayload(auxBuf);
            //int payload = getLastMessageId(payloadBuf);
            if (aux_equipment_id == equipment_id)
            {
                printf("Successful removal\n");
                bzero(buf,256);
                bzero(auxBuf,256);
                bzero(payloadBuf,256);
                bzero(dataBuf,256);
                break;
            } else {
                if (aux_equipment_id < 10)
                {
                    printf("Equipment 0%d removed\n", aux_equipment_id + 1);
                } else {
                    printf("Equipment %d removed\n", aux_equipment_id + 1);
                }
            }
        } else {
            printf("Received: %s\n", auxBuf);
        }
        bzero(buf,256);
        bzero(auxBuf,256);
        bzero(payloadBuf,256);
        bzero(dataBuf,256);
    }

    close(s);
    exit(EXIT_SUCCESS);

}