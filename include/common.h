#ifndef COMMON_H
#define COMMON_H

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
#include <netinet/tcp.h>
#include <netdb.h>

#include "http.h"

#ifndef ARM9
    #define iprintf(...) printf(__VA_ARGS__)
    #define closesocket(fd) close(fd)
#endif

int get_line(int sockfd, char* buf, int len);

void send_header(int sockfd, const char* key, const char* value);
void send_body(int sockfd, const char* body, int len);

#endif
