#include "client.h"

int http_request(const char* method, const char* host, const char* request, const char* body, int bodylen, char** resp, int* resplen) {
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(HTTP_PORT);
    if (inet_aton(host, &addr.sin_addr) == 0) {
        struct hostent* he = gethostbyname(host);
        if (he == NULL) {
            return 0;
        }
        addr.sin_addr.s_addr = *((unsigned long*)(he->h_addr_list[0]));
    }

    int sockfd = socket(PF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        return 0;
    }

    #ifdef TCP_NODELAY
        if (setsockopt(sockfd, SOL_TCP, TCP_NODELAY, &(int){ 1 }, sizeof(int)) < 0) {
            iprintf("[!] Could not set nodelay option!");
        }
    #endif

    #ifdef TCP_CORK
        if (setsockopt(sockfd, SOL_TCP, TCP_CORK, &(int){ 1 }, sizeof(int)) < 0) {
            iprintf("[!] Could not set cork option!");
        }
    #endif

    #ifdef TCP_QUICKACK
        if (setsockopt(sockfd, SOL_TCP, TCP_QUICKACK, &(int){ 1 }, sizeof(int)) < 0) {
            iprintf("[!] Could not set cork option!");
        }
    #endif

    if (connect(sockfd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        closesocket(sockfd);
        return 0;
    }

    send_request(sockfd, method, request);
    send_header(sockfd, HOST, host);
    send_body(sockfd, body, bodylen);

    int status = 0;
    char* line = (char*)malloc(1024 * 4);
    if (get_line(sockfd, line, 1024) > 0) {
        char* version = (char*)malloc(16);
        char* status_text = (char*)malloc(128);
        if (sscanf(line, "%15s %d %127s", version, &status, status_text) == 3 && resp != NULL && resplen != NULL) {
            *resplen = 0;
            char* key = (char*)malloc(32);
            char* value = (char*)malloc(128);
            while (get_line(sockfd, line, 1024 * 4) > 2) {
                if (sscanf(line, "%31[^:]: %127s", key, value) > 0) {
                    if (strcmp(key, CONTENT_LENGTH) == 0) {
                        sscanf(value, "%d", resplen);
                    }
                }
            }
            free(key);
            free(value);

            int read = 0;
            if (*resplen > 0) {
                int offset = 0;
                *resp = (char*)malloc(*resplen + 1);
                while (offset < *resplen && (read = recv(sockfd, &(*resp)[offset], *resplen - offset, 0)) >= 0) {
                    offset += read;
                }
                (*resp)[offset] = 0;
                *resplen = offset;
            } else {
                *resp = (char*)malloc(1025);
                while (*resp != NULL && (read = recv(sockfd, &(*resp)[*resplen], 1024, 0)) > 0) {
                    *resplen += read;
                    (*resp)[*resplen] = 0;
                    *resp = (char*)realloc(*resp, *resplen + 1025);
                }
            }
        }
        free(version);
        free(status_text);
    }
    free(line);

    shutdown(sockfd, SHUT_RDWR);
    closesocket(sockfd);

    return status;
}

int http_get(const char* host, const char* request, char** resp, int* resplen) {
    return http_request(GET, host, request, NULL, 0, resp, resplen);
}

void send_request(int sockfd, const char* method, const char* request) {
    char* msg = (char*)malloc(1024);
    sprintf(msg, "%s %s HTTP/1.0\r\n", method, request);
    send(sockfd, msg, strlen(msg), 0);
    free(msg);
}
