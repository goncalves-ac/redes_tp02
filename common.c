#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <arpa/inet.h>

void ERROR(const char *msg) {
    perror(msg);
    exit(1);
}
