#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#include "common.h"

int countID = 0;
int equipament[15] = {-1};
int equipament_ids[15] = {-1};

int main(int argc, char *argv[]) {
    int sockfd, newsockfd, portno;
    socklen_t clilen;
    char buffer[256];
    struct sockaddr_in serv_addr, cli_addr;
    int n;
    if (argc < 2) {
         fprintf(stderr, "ERROR, no port provided\n");
         exit(1);
    }
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
        error("ERROR opening socket");
    bzero((char *) &serv_addr, sizeof(serv_addr));
    portno = atoi(argv[1]);
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portno);
    if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
        error("ERROR on binding");
    listen(sockfd, 5);
    clilen = sizeof(cli_addr);
    //Below code is modified to handle multiple clients using fork
    //------------------------------------------------------------------
    int pid;
    int cnt = 0;
    while (1) {
         newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
         if (newsockfd < 0)
                error("ERROR on accept");
         //fork new process
         pid = fork();
         if (pid < 0) {
            error("ERROR in new process creation");
         }
         if (pid == 0) {
            //child process
            //do whatever you want
            
            cnt++;
            bzero(buffer, 256);
            n = read(newsockfd, buffer, 255);
            if (n < 0)
                error("ERROR reading from socket");
            printf("Here is the message: %s\n", buffer);
            if (strcmp(buffer, "close connection\n") == 0 || strcmp(buffer, "close connection") == 0)
            {
                n = write(newsockfd, "Successful removal", 18);
            } else {
                char s[100];
                char newChar[100];
                sprintf(newChar, "%d", cnt);
                memccpy(memccpy(s, "I got your message ", '\0', 100) - 1, newChar, '\0', 100);
                n = write(newsockfd, s, 50);
            }
           
            if (n < 0)
                 error("ERROR writing to socket");
            close(newsockfd);
        } else {
            //parent process
            close(newsockfd);
        }
    }
    //-------------------------------------------------------------------
   return 0;
}
