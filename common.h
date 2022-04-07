#pragma once

#include <stdlib.h>

#include <arpa/inet.h>

#define WELCOME_CODE 1
#define GUESS_CODE 2
#define ANSWER_CODE 3
#define FINAL_CODE 4


void logexit(const char *msg);

int addrparse(const char *addrstr, const char *portstr,
              struct sockaddr_storage *storage);

void addrtostr(const struct sockaddr *addr, char *str, size_t strsize);

int server_sockaddr_init(const char *proto, const char *portstr,
                         struct sockaddr_storage *storage);
