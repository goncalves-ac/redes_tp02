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
#include <pthread.h>

#define PORT 5555
#define MAXMSG 512


void *connection_handler(void *socket_desc)
{
    int sock = *(int*)socket_desc;
    int read_size;
    char *message, client_message[2000], *message_to_client;
    char broadcast[50] = "new client connected";
    int num;

    num = sock;
    num--;
    while(num > 3)
    {
        write(num, broadcast, strlen(broadcast));
        sleep(1);
        num--;
    }

    while((read_size = recv(sock, client_message, 2000, 0)) > 0)
    {
       client_message[read_size] = '\0';
       printf("Client[%d]: %s", sock, client_message);
       message_to_client = "I hear you dude...";
       write(sock, message_to_client, strlen(message_to_client));
       memset(client_message, 0, 2000);
    }

    if(read_size == 0)
    {
        printf("Client[%d] disconnected", sock);
        fflush(stdout);
    }
    else if(read_size == -1)
    {
        perror("recv failed");

    }

    free(socket_desc);
    return 0;
} 

int makeSocket(unsigned short int port) {
    int sock;
    struct sockaddr_in name;
    /* Create a socket. */
    sock = socket(PF_INET, SOCK_STREAM, 0);
    if(sock < 0) {
        perror("Could not create a socket\n");
        exit(EXIT_FAILURE);
    }
    name.sin_family = AF_INET;
    name.sin_port = htons(port);
    name.sin_addr.s_addr = htonl(INADDR_ANY);
    if(bind(sock, (struct sockaddr *)&name, sizeof(name)) < 0) {
        perror("Could not bind a name to the socket\n");
        exit(EXIT_FAILURE);
    }
    return(sock);
}

int main(int argc, char *argv[])
{
    int sock;
    int clientSocket;
    int i;
    int *new_sock;
    char *broadcast;
    fd_set activeFdSet, readFdSet;
    struct sockaddr_in clientName;
    socklen_t size;
    sock = makeSocket(PORT);
    if(listen(sock,1) < 0) 
    {
        perror("Could not listen for connections\n");
        exit(EXIT_FAILURE);
    }
    FD_ZERO(&activeFdSet);
    FD_SET(sock, &activeFdSet);
    while(1) 
    {
        printf("Waiting for connections\n");
        readFdSet = activeFdSet;
        if(select(FD_SETSIZE, &readFdSet, NULL, NULL, NULL) < 0)
        {
            perror("Select failed\n");
            exit(EXIT_FAILURE);
        }
        for(i = 0; i < FD_SETSIZE; ++i)
        {
            if(FD_ISSET(i, &readFdSet)) 
            {
                if(i == sock) 
                {
                    size = sizeof(struct sockaddr_in);
                    pthread_t sniffer_thread;
                    while(( clientSocket = accept(sock, (struct sockaddr *)&clientName, (socklen_t *)&size))) 
                    {
                        puts("Connection accepted");
                        new_sock = malloc(1);
                        *new_sock = clientSocket;
                        if( pthread_create( &sniffer_thread, NULL, connection_handler, (void*) new_sock) < 0)
                        {
                            perror("could not create thread");
                            return 1;
                        }
                        broadcast = "NEW CLIENT CONNECTED";
                        write(4, broadcast, sizeof(broadcast)); //just to see if when ever a new client connects the first client should get all these messages
                        write(4, broadcast, sizeof(broadcast));
                        write(4, broadcast, sizeof(broadcast));
                        pthread_detach(sniffer_thread);
                        puts("Handler assigned");
                        FD_SET(*new_sock, &activeFdSet);

                   }
                   if(clientSocket < 0) 
                   {
                       perror("Could not accept connection\n");
                       exit(EXIT_FAILURE);
                   }                
                }
                else 
                {
                    if(readMessageFromClient(i) < 0)
                    {
                    close(i);
                        FD_CLR(i, &activeFdSet);
                    }
                }
            }
        }      
    }
} 