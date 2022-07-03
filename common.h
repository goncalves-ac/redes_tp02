//
// Created by thimorais
//
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

struct equipment_data equipment[CLIENTS];

int equipment_client[CLIENTS];

int countEquipment;

pthread_mutex_t lock;


void clientUsage(int argc, char **argv);

void serverUsage(int argc, char **argv);

void logExit(const char *msg);

int addrparse(const char *addrstr, const char *portstr, struct sockaddr_storage *storage);

void addrtostr(const struct sockaddr *addr, char *str, size_t strsize);

int server_sockaddr_init(const char *proto, const char *portstr, struct sockaddr_storage *storage);

char *recuperarIdMensagem(char *string);

int recuperarIdEquipamentoDestino(char *string); // get equipment id with payload from string

int getLastMessageId(char *string); // get last message id from string

int getTargetId(char *string); // get target id from string

void printDataValue(char *string); // print data value from string

float geradorLeituraAleatoria(int min, int max); // generate random float

void broadcast(char *string, int unique_id, struct equipment_data *equipment,
               pthread_mutex_t lock); // send message to all clients

void *client_thread(void *data);