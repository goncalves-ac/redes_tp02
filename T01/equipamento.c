#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 

#include "common.h"

int main(int argc, char *argv[])
{
    int sockfd, portno, n;
    struct sockaddr_in serv_addr;
    struct hostent *server;

    int equipment[15] = {-1};
    int equipment_id = -1;

    char buffer[256];
    char auxBuffer[256];
    while(1){
        if (argc < 3) {
            fprintf(stderr,"usage %s hostname port\n", argv[0]);
            exit(0);
        }
        portno = atoi(argv[2]);
        sockfd  = socket(AF_INET, SOCK_STREAM, 0);
        if (sockfd < 0) 
            ERROR("ERROR opening socket");
        server = gethostbyname(argv[1]);
        if (server == NULL) {
            fprintf(stderr,"ERROR, no such host\n");
            exit(0);
        }
        bzero((char *) &serv_addr, sizeof(serv_addr));
        serv_addr.sin_family = AF_INET;
        bcopy((char *)server->h_addr, 
            (char *)&serv_addr.sin_addr.s_addr,
            server->h_length);
        serv_addr.sin_port = htons(portno);
        if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) 
            ERROR("ERROR connecting");

        if (equipment_id == -1){
             n = write(sockfd,"01",strlen("01"));
        } else {
            printf("Please enter the message: ");
            bzero(buffer,256);
            fgets(buffer,255,stdin);
            n = write(sockfd,buffer,strlen(buffer));
        }
        if (n < 0) 
            ERROR("ERROR writing to socket");

        bzero(buffer,256);
        n = read(sockfd,buffer,255);
        if (n < 0) 
            ERROR("ERROR reading from socket");
        
        sprintf(auxBuffer,"%s",buffer);
        if (strcmp(getMessageId(buffer), "03") == 0)
        {
            equipment_id = getEquipmentId(auxBuffer);
            equipment[equipment_id] = 0;
            printf("New ID: %d\n", equipment_id + 1);
        }
        if (strcmp(buffer, "Successful removal\n") == 0 || strcmp(buffer, "Successful removal") == 0)
        {
            bzero(buffer,256);
            close(sockfd);
            return 0;
        }
        
    }
}