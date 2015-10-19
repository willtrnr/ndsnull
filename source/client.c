#include "client.h"

int http_request(const char* method, const char* host, const char* request, const char* body, int bodylen, char** resp, int* resplen) {
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(80);
    if (inet_aton(host, &addr.sin_addr) == 0) {
        struct hostent* he = gethostbyname(host);
        if (he == NULL) {
            return 0;
        }
        addr.sin_addr.s_addr = *((unsigned long*)(he->h_addr_list[0]));
    }

    int sockfd = socket(PF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        return 0;
    }
    if (connect(sockfd, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
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
            while (get_line(sockfd, line, 1024 * 4) > 2);

            int i = 0;
            *resp = (char*)malloc(1024);
            while ((i = recv(sockfd, &(*resp)[*resplen], 1024, 0)) > 0) {
                *resplen += i;
                char* ex = (char*)realloc(*resp, *resplen + 1024);
                if (ex == NULL) {
                    break;
                }
                *resp = ex;
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

void send_request(int sockfd, const char* method, const char* request) {
    char* msg = (char*)malloc(1024);
    sprintf(msg, "%s %s HTTP/1.0\r\n", method, request);
    send(sockfd, msg, strlen(msg), 0);
    free(msg);
}
