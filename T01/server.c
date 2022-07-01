#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/shm.h>

#include "common.h"

int countID = 0;
int *equipment;

int main(int argc, char *argv[]) {
    int sockfd, newsockfd, portno;
    socklen_t clilen;
    char buffer[256];
    int shmid;
    struct sockaddr_in serv_addr, cli_addr; 

    int n;
    if (argc < 2) {
         fprintf(stderr, "ERROR, no port provided\n");
         exit(1);
    }
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
        herror("ERROR opening socket");
    bzero((char *) &serv_addr, sizeof(serv_addr));
    portno = atoi(argv[1]);
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portno);
    if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
        herror("ERROR on binding");

    listen(sockfd, 5);
    clilen = sizeof(cli_addr);
    //Below code is modified to handle multiple clients using fork
    //------------------------------------------------------------------

    if ((shmid = shmget((key_t) 5555, (15*sizeof(int)), IPC_CREAT | 0666)) < 0)
    {
        printf("Cannot shmget");
        herror("Cannot shmget");
    }
    
    if ((equipment = (int *)shmat(shmid, 1, 0)) < (int *)0)
    {
        printf("Cannot shmat");
        herror("Cannot shmat");
    }

    int pid;
    int cnt = 0;
    while (1) {
         newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
         countID++;
         if (newsockfd < 0)
                herror("ERROR on accept");
         //fork new process
         pid = fork();
         if (pid < 0) {
            herror("ERROR in new process creation");
         }
         if (pid == 0) {
            //child process
            //do whatever you want
            
            cnt++;
            bzero(buffer, 256);
            n = read(newsockfd, buffer, 255);
            if (n < 0)
                herror("ERROR reading from socket");
            printf("Here is the message: %s\n", buffer);
            if (strcmp(buffer, "close connection\n") == 0 || strcmp(buffer, "close connection") == 0)
            {
                countID--;
                n = write(newsockfd, "Successful removal", 18);
            } 
            
            if (strcmp(buffer, "01\n") == 0 || strcmp(buffer, "01") == 0)
            {

                for (int i = 0; i < 15; i++)
                {
                    if(equipment[i] == -1)
                    {
                        equipment[i] = 1;
                        char s[20];
                        char id[10];
                        sprintf(id, "%d", i);
                        memccpy(memccpy(s, "03 ", '\0', 20) - 1, id, '\0', 20);
                        n = write(newsockfd, s, 18);
                        break;
                    }
                }

            } else {
                char s[100];
                char newChar[100];
                sprintf(newChar, "%d", cnt);
                memccpy(memccpy(s, "I got your message ", '\0', 100) - 1, newChar, '\0', 100);
                n = write(newsockfd, s, 50);
            }
           
            if (n < 0)
                herror("ERROR writing to socket");
            close(newsockfd);
        } else {
            //parent process
            close(newsockfd);
        }
    }
    //-------------------------------------------------------------------
   return 0;
}
