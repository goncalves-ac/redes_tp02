#pragma once

#include <stdlib.h>
#include <arpa/inet.h>

void ERROR(const char *msg);

void req_add();

void resp_add();

char* getMessageId(char* string);

int getEquipmentId(char* string);
