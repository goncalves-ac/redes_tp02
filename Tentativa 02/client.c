#include <pthread.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/times.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string.h>

#define PORT 5555
#define hostNameLength 50
#define messageLength  256

void initSocketAddress(struct sockaddr_in *name, char *hostName, unsigned short int port) 
{
  struct hostent *hostInfo; 
  name->sin_family = AF_INET;
  name->sin_port = htons(port);
  hostInfo = gethostbyname(hostName);
  if(hostInfo == NULL)
  {
    fprintf(stderr, "initSocketAddress - Unknown host %s\n",hostName);
    exit(EXIT_FAILURE);
  }
  name->sin_addr = *(struct in_addr *)hostInfo->h_addr;
}
void writeMessage(int fileDescriptor, char *message) 
{
  int nOfBytes;
  nOfBytes = write(fileDescriptor, message, strlen(message) + 1);
  if(nOfBytes < 0) 
  {
    perror("writeMessage - Could not write data\n");
    exit(EXIT_FAILURE);
  }
}

void * receiveMessage(void * socket)
{
    int sockfd;
    //int ret;
    char buffer[1024];
    sockfd = (int) socket;
    //int count = 1;
     memset(buffer, 0, 1024);
     for (;;)
     {
         if (recvfrom(sockfd, buffer, 1024, 0, NULL, NULL) > 0)
         {
             fputs(buffer, stdout);
             printf("\n");
         }
     }
}

int main(int argc, char *argv[]) 
{
  int sock;
  struct sockaddr_in serverName;
  char hostName[hostNameLength];
  char messageString[messageLength];
  char buffer[1024];
  if(argv[1] == NULL) 
  {
    perror("Usage: client [host name]\n");
    exit(EXIT_FAILURE);
  }
  else 
  {
    strncpy(hostName, argv[1], hostNameLength);
    hostName[hostNameLength - 1] = '\0';
  }

  //creating a new thread for receiving messages from the server
   read = pthread_create(&pthread, NULL, receiveMessage, (void *) sock);
   if (read < 0)
   {
       printf("ERROR: Return Code from pthread_create() is %d\n", read);
       exit(1);
   }

  sock = socket(PF_INET, SOCK_STREAM, 0);
  if(sock < 0) 
  {
    perror("Could not create a socket\n");
    exit(EXIT_FAILURE);
  }

  initSocketAddress(&serverName, hostName, PORT);
  if(connect(sock, (struct sockaddr *)&serverName, sizeof(serverName)) < 0) 
  {
    perror("Could not connect to server\n");
    exit(EXIT_FAILURE);
  }
  printf("\nType something and press [RETURN] to send it to the server.\n");
  printf("Type 'quit' to nuke this program.\n");
  fflush(stdin);
  recv(sock, buffer, 1024, 0);
  printf(buffer);
  while(1) 
  while(1)
  {
      printf("\n>");

      fgets(messageString, messageLength, stdin);

      messageString[messageLength - 1] = '\0';

      if(strncmp(messageString, "quit\n",messageLength) != 0)
      {
          writeMessage(sock, messageString);
      }
  }
}

