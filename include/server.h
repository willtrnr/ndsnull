#ifndef SERVER_H
#define SERVER_H

#ifdef NDS
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
#include <netinet/tcp.h>
#include <netdb.h>

#include "config.h"
#include "http.h"
#include "index.h"
#include "common.h"
#include "client.h"
#include "json.h"
#include "json-builder.h"

int http_server(int port);
void accept_client(int sockfd);
void process_request(int sockfd, const char* method, const char* request, const char* body, int bodylen);

void send_status(int sockfd, int status);

#endif
