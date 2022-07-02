#include "common.h"

#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <arpa/inet.h>

void logExit(const char *msg) {
	perror(msg);
	exit(EXIT_FAILURE);
}

int addrparse(const char *addrstr, const char *portstr,
              struct sockaddr_storage *storage) {
    if (addrstr == NULL || portstr == NULL) { // verifica se os argumentos são nulos
        return -1;
    }

    uint16_t port = (uint16_t)atoi(portstr); // unsigned short
    if (port == 0) {
        return -1;
    }
    port = htons(port); // host to network short

    struct in_addr inaddr4; // 32-bit IP address
    if (inet_pton(AF_INET, addrstr, &inaddr4)) {
        struct sockaddr_in *addr4 = (struct sockaddr_in *)storage;
        addr4->sin_family = AF_INET;
        addr4->sin_port = port;
        addr4->sin_addr = inaddr4;
        return 0;
    }

    struct in6_addr inaddr6; // 128-bit IPv6 address
    if (inet_pton(AF_INET6, addrstr, &inaddr6)) {
        struct sockaddr_in6 *addr6 = (struct sockaddr_in6 *)storage;
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
        struct sockaddr_in *addr4 = (struct sockaddr_in *)addr; // IPv4 address
        if (!inet_ntop(AF_INET, &(addr4->sin_addr), addrstr,
                       INET6_ADDRSTRLEN + 1)) { // IPv4 address to string
            logExit("ntop");
        }
        port = ntohs(addr4->sin_port); // network to host short
    } else if (addr->sa_family == AF_INET6) {
        version = 6;
        struct sockaddr_in6 *addr6 = (struct sockaddr_in6 *)addr; // IPv6 address
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
    uint16_t port = (uint16_t)atoi(portstr); // unsigned short
    if (port == 0) {
        return -1;
    }
    port = htons(port); // host to network short

    memset(storage, 0, sizeof(*storage));
    if (0 == strcmp(proto, "v4")) {
        struct sockaddr_in *addr4 = (struct sockaddr_in *)storage;
        addr4->sin_family = AF_INET; // IPv4
        addr4->sin_addr.s_addr = INADDR_ANY; // any interface
        addr4->sin_port = port; // port
        return 0;
    } else if (0 == strcmp(proto, "v6")) {
        struct sockaddr_in6 *addr6 = (struct sockaddr_in6 *)storage;
        addr6->sin6_family = AF_INET6; // IPv6
        addr6->sin6_addr = in6addr_any; // any interface
        addr6->sin6_port = port;// port
        return 0;
    } else {
        return -1;
    }
}

void req_add() {
    printf("01");
}

char* getMessageId(char* string) {
    // return the message id from to print the message
    return strtok(string, " ");
}

int getLastMessageId(char* string) {
    // return the last message id from to print the message
    char * token = strtok(string, " "); // get the first token, this case id of the message
    int id = -1;
    while( token != NULL ) {
        if( token != NULL) {
            id = atoi(token); // convert the token to int, to know the id message or equipment
        }
        token = strtok(NULL, " "); // get the next token
    }

    return id;
}

void printDataValue(char* string) {
    // return the data value from to print the message when need to information from equipment
    char * token = strtok(string, " "); // get the first token, this case id of the message
    char id[20];
    while( token != NULL ) {
        if( token != NULL) {
            sprintf(id, "%s", token); 
        }
        token = strtok(NULL, " "); // get the next token
    }

    printf("%s\n", id);
}

int getTargetId(char* string) {
    // returns the device id on which to return the requested information 
    char * token = strtok(string, " "); // get the first token
    int id = -1;
    int count = 1;
    while( token != NULL ) {
        if( token != NULL && count == 3) { // this this case de position of the device id is 3, always - the according to the issue specification.
            id = atoi(token);
        }
        token = strtok(NULL, " ");
        count++;
    }

    return id;
}

int getEquipmentIdWithPayload(char* string) {
    // return the id of the equipment where you need to send the payload
    char * token = strtok(string, " ");
    int id = -1;
    int count = 1;
    while( token != NULL ) {
        if( token != NULL && count == 2) { // this this case de position of the device id is 2, always - the according to the issue specification.
            id = atoi(token);
        }
        token = strtok(NULL, " ");
        count++;
    }

    return id;
}

float rand_float(int min, int max) {
    // Generate random numbers
    float scale = rand() / (float)RAND_MAX; // [0, 1.0]
    return min + scale * (max - min); // [min, max]
}

void send_message_to_all(char *string, int unique_id, struct equipment_data *equipment , pthread_mutex_t lock) {
	pthread_mutex_lock(&lock);
    // Send the message to all clients except the sender
	for(int i=0; i< CLIENTS; ++i){
		if(equipment[i].equipment_used != -1){
			if(equipment[i].eq_sock != unique_id){
                char aux[100];
                sprintf(aux, "%s", string); // copy the string to a new one, to avoid to modify the original one
                memccpy(string, aux, '\0', 100); // copy the string to the assistant string, to avoid the memory leak
                int id = equipment[i].eq_sock; // get the id of the device to which the message should be forwarded
                //size_t count = send(equipment[i].eq_sock, string, strlen(string) + 1, 0);
                size_t count = write(id, string, strlen(string)+1); // send the message to the equipment

				if(count < 0){
					printf("ERROR: write to descriptor failed\n");
				}
			}
		}
	}
    
	pthread_mutex_unlock(&lock);
}
