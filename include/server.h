#ifndef SERVER_H
#define SERVER_H

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
#include "payload.h"
#include "common.h"
#include "client.h"

void accept_client(int sockfd);
void process_request(int sockfd, const char* method, const char* request, const char* body, int bodylen);

void send_status(int sockfd, int status);

#endif
