#pragma once

#include <stdlib.h>
#include <arpa/inet.h>

#define BUFSIZE 1024
#define CLIENTS 15

struct client_data {
    int csock;
    struct sockaddr_storage storage;
};

struct equipment_data {
    int csock;
    int equipment_used;
};

void logExit(char *msg);

int addrparse(const char *addrstr, const char *portstr, struct sockaddr_storage *storage);

void addrtostr(const struct sockaddr *addr, char *str, size_t strsize);

int server_sockaddr_init(const char *proto, const char *portstr, struct sockaddr_storage *storage);

void req_add();

char* getMessageId(char* string);

int getEquipmentIdWithPayload(char* string);

int getLastMessageId(char* string);

int getTargetId(char* string);

void printDataValue(char* string);

float rand_float(int min, int max);