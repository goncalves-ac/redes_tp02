//
// Created by thimorais
//
#pragma once

#include <stdlib.h>
#include <arpa/inet.h>

#define BUFSIZE 1024
#define MAXEQUIPMENT 15


struct client_data {
    int csock;
    struct sockaddr_storage storage;

};

struct equipment_data {
    int eq_sock;
    int equipment_used;
};

struct equipment_data equipment[MAXEQUIPMENT];

int equipment_client[MAXEQUIPMENT];

int countEquipment;

pthread_mutex_t lock;


void clientUsage(int argc, char **argv);

void serverUsage(int argc, char **argv);

void logExit(const char *msg);

int addrparse(const char *addrstr, const char *portstr, struct sockaddr_storage *storage);

void addrtostr(const struct sockaddr *addr, char *str, size_t strsize);

int server_sockaddr_init(const char *proto, const char *portstr, struct sockaddr_storage *storage);

char *recuperarIdMensagem(char *string);

int recuperarIdEquipamentoDestino(char *string);

int recuperarIdUltimaMensagem(char *string);

int recuperarIdDestino(char *string);

void printDataValue(char *string);

float geradorLeituraAleatoria(int min, int max);

void broadcast(char *string, int unique_id, struct equipment_data *equipment, pthread_mutex_t lock);

void *client_thread(void *data);