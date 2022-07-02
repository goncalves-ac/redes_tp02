#pragma once

#include <stdlib.h>
#include <arpa/inet.h>

#define BUFSIZE 1024
#define CLIENTS 15

struct client_data { // client data structure
    int csock; // client socket
    struct sockaddr_storage storage; // client address

};

struct equipment_data { // equipment
    int eq_sock; // equipment socket
    int equipment_used; // equipment is used is 1, not used is -1
};

void logExit(const char *msg); // log message and exit

int addrparse(const char *addrstr, const char *portstr, struct sockaddr_storage *storage); // parse address and port

void addrtostr(const struct sockaddr *addr, char *str, size_t strsize); // convert address to string

int server_sockaddr_init(const char *proto, const char *portstr, struct sockaddr_storage *storage); // init server address

void req_add(); // add requester

char* getMessageId(char* string); // get message id from string

int getEquipmentIdWithPayload(char* string); // get equipment id with payload from string

int getLastMessageId(char* string); // get last message id from string

int getTargetId(char* string); // get target id from string

void printDataValue(char* string); // print data value from string

float rand_float(int min, int max); // generate random float

void send_message_to_all(char *string, int unique_id, struct equipment_data *equipment, pthread_mutex_t lock); // send message to all clients
