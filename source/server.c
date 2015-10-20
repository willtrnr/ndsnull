#include "server.h"

int http_server(int port) {
    int servfd = socket(PF_INET, SOCK_STREAM, 0);
    if (servfd == -1) {
        iprintf("[!] Could not create socket!");
        return 1;
    }

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(HTTP_PORT);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(servfd, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
        closesocket(servfd);
        iprintf("[!] Could not bind to port %d!", HTTP_PORT);
        return 1;
    }

    if (listen(servfd, 10) == -1) {
        closesocket(servfd);
        iprintf("[!] Could not listen on socket!");
        return 1;
    }

    iprintf("[*] Listening on %s:%d...\n", inet_ntoa(addr.sin_addr), HTTP_PORT);
    for (;;) accept_client(servfd);
}

void accept_client(int servfd) {
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    int addrlen = 0;
    int sockfd = accept(servfd, (struct sockaddr*)&addr, &addrlen);
    if (sockfd == -1) {
        iprintf("[!] Could not accept client!\n");
        return;
    }

    iprintf("[<] Client connected: %s\n", inet_ntoa(addr.sin_addr));

    char* line = (char*)malloc(1024 * 4);
    if (get_line(sockfd, line, 1024) > 0) {
        char* method = (char*)malloc(16);
        char* request = (char*)malloc(256);
        char* version = (char*)malloc(16);
        if (sscanf(line, "%15s %255s %15s", method, request, version) == 3) {
            int bodylen = 0;
            char* key = (char*)malloc(32);
            char* value = (char*)malloc(128);
            while (get_line(sockfd, line, 1024 * 4) > 2) {
                if (sscanf(line, "%31[^:]: %127s", key, value) > 0) {
                    if (strcmp(key, CONTENT_LENGTH) == 0) {
                        sscanf(value, "%d", &bodylen);
                    }
                }
            }
            free(key);
            free(value);

            if (bodylen > MAX_BODY_SIZE) {
                send_status(sockfd, REQUEST_ENTITY_TOO_LARGE);
                send_body(sockfd, NULL, 0);
                iprintf("[>] %d\n", REQUEST_ENTITY_TOO_LARGE);
            } else {
                char* body = NULL;
                if (bodylen > 0) {
                    int read = 0;
                    int offset = 0;
                    body = (char*)malloc(bodylen + 1);
                    while (offset < bodylen && (read = recv(sockfd, &body[offset], bodylen - offset, 0)) >= 0) {
                        offset += read;
                    }
                    body[offset] = 0;
                    bodylen = offset;
                }
                process_request(sockfd, method, request, body, bodylen);
                free(body);
            }
        } else {
            send_status(sockfd, BAD_REQUEST);
            send_body(sockfd, NULL, 0);
            iprintf("[>] %d\n", BAD_REQUEST);
        }

        free(method);
        free(request);
        free(version);
    } else {
        send_status(sockfd, BAD_REQUEST);
        send_body(sockfd, NULL, 0);
        iprintf("[>] %d\n", BAD_REQUEST);
    }
    free(line);

    shutdown(sockfd, SHUT_RDWR);
    closesocket(sockfd);
}

void process_request(int sockfd, const char* method, const char* request, const char* body, int bodylen) {
    if (strcmp(method, GET) == 0) {
        if (strcmp(request, "/") == 0) {
            send_status(sockfd, OK);
            send_header(sockfd, CONTENT_TYPE, "text/html");
            send_body(sockfd, INDEX, sizeof(INDEX));
            iprintf("[>] %d GET %s\n", OK, request);
        } else if (strcmp(request, "/google") == 0) {
            char* resp = NULL;
            int resplen = 0;
            int status = http_get("www.google.ca", "/", &resp, &resplen);
            send_status(sockfd, status);
            send_body(sockfd, resp, resplen);
            free(resp);
            iprintf("[>] %d GET %s\n", status, request);
        } else {
            // send_status(sockfd, NOT_FOUND);
            // send_body(sockfd, NULL, 0);
            // iprintf("[>] 404 GET %s\n", request);
            char* resp = NULL;
            int resplen = 0;
            int status = http_get("www.google.ca", request, &resp, &resplen);
            send_status(sockfd, status);
            send_body(sockfd, resp, resplen);
            free(resp);
            iprintf("[>] %d GET %s\n", status, request);
        }
    } else if (strcmp(method, POST) == 0) {
        if (strcmp(request, "/") == 0) {
            int firstNumber = 0;
            int secondNumber = 0;
            sscanf(body, "firstNumber=%d&secondNumber=%d", &firstNumber, &secondNumber);

            send_status(sockfd, OK);
            send_header(sockfd, CONTENT_TYPE, "application/json");
            char* msg = (char*)malloc(sizeof(PAYLOAD) + 32);
            sprintf(msg, PAYLOAD, firstNumber + secondNumber);
            send_body(sockfd, msg, strlen(msg));
            free(msg);

            iprintf("[>] %d POST %s\n", OK, request);
        } else {
            send_status(sockfd, NOT_FOUND);
            send_body(sockfd, NULL, 0);
            iprintf("[>] %d GET %s\n", NOT_FOUND, request);
        }
    } else {
        send_status(sockfd, NOT_IMPLEMENTED);
        send_body(sockfd, NULL, 0);
        iprintf("[>] %d %s %s\n", NOT_IMPLEMENTED, method, request);
    }
}

void send_status(int sockfd, int status) {
    if (status == 0) {
        status = INTERNAL_SERVER_ERROR;
    }
    char* msg = (char*)malloc(64);
    sprintf(msg, "HTTP/1.0 %d ", status);
    switch (status) {
        case OK:
            strcat(msg, "OK");
            break;
        case CONTINUE:
            strcat(msg, "Continue");
            break;
        case BAD_REQUEST:
            strcat(msg, "Bad Request");
            break;
        case NOT_FOUND:
            strcat(msg, "Not Found");
            break;
        case REQUEST_ENTITY_TOO_LARGE:
            strcat(msg, "Request Entity Too Large");
            break;
        case INTERNAL_SERVER_ERROR:
            strcat(msg, "Internal Server Error");
            break;
        case NOT_IMPLEMENTED:
            strcat(msg, "Not Implemented");
            break;
    }
    strcat(msg, "\r\n");
    send(sockfd, msg, strlen(msg), 0);
    send_header(sockfd, SERVER, "/nds/null v4.2.0");
    free(msg);
}
