#ifndef CLIENT_H
#define CLIENT_H

#ifdef ARM9
    #include <nds.h>
#else
    #include <stdlib.h>
    #include <unistd.h>
    #include <arpa/inet.h>
#endif
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#include "config.h"
#include "http.h"
#include "common.h"

int http_request(const char* method, const char* host, const char* request, const char* body, int bodylen, char** resp, int* resplen);
int http_get(const char* host, const char* request, char** resp, int* resplen);

void send_request(int sockfd, const char* method, const char* request);

#endif
