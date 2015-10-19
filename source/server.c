#include "server.h"

void accept_client(int sockfd) {
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    int addrlen = 0;
    int clientfd = accept(sockfd, (struct sockaddr*)&addr, &addrlen);
    if (clientfd == -1) {
        iprintf("[!] Could not accept client!\n");
        return;
    }

    iprintf("[<] Client connected: %s\n", inet_ntoa(addr.sin_addr));

    char* line = (char*)malloc(1024 * 4);
    if (get_line(clientfd, line, 1024) > 0) {
        char* method = (char*)malloc(16);
        char* request = (char*)malloc(256);
        char* version = (char*)malloc(16);
        if (sscanf(line, "%15s %255s %15s", method, request, version) == 3) {
            int bodylen = 0;
            char* key = (char*)malloc(32);
            char* value = (char*)malloc(128);
            while (get_line(clientfd, line, 1024 * 4) > 2) {
                if (sscanf(line, "%31[^:]: %127s", key, value) > 0) {
                    if (strcmp(key, CONTENT_LENGTH) == 0) {
                        sscanf(value, "%d", &bodylen);
                    }
                }
            }
            free(key);
            free(value);

            if (bodylen > MAX_BODY_SIZE) {
                send_status(clientfd, REQUEST_ENTITY_TOO_LARGE);
                send_body(clientfd, NULL, 0);
                iprintf("[>] 400\n");
            } else {
                char* body = NULL;
                if (bodylen > 0) {
                    body = (char*)malloc(bodylen);
                    bodylen = recv(clientfd, body, bodylen, 0);
                }

                process_request(clientfd, method, request, body, bodylen);

                free(body);
            }
        } else {
            send_status(clientfd, BAD_REQUEST);
            send_body(clientfd, NULL, 0);
            iprintf("[>] 400\n");
        }

        free(method);
        free(request);
        free(version);
    } else {
        send_status(clientfd, BAD_REQUEST);
        send_body(clientfd, NULL, 0);
        iprintf("[>] 400\n");
    }
    free(line);

    shutdown(clientfd, SHUT_RDWR);
    closesocket(clientfd);
}

void process_request(int sockfd, const char* method, const char* request, const char* body, int bodylen) {
    if (strcmp(method, GET) == 0) {
        if (strcmp(request, "/") == 0) {
            send_status(sockfd, OK);
            send_header(sockfd, CONTENT_TYPE, "text/html");
            send_body(sockfd, INDEX, sizeof(INDEX));
            iprintf("[>] 200 GET %s\n", request);
        } else if (strcmp(request, "/google") == 0) {
            char* resp = NULL;
            int resplen = 0;
            int status = http_request("www.google.ca", "/", NULL, 0, &resp, &resplen);
            send_status(sockfd, status);
            send_body(sockfd, resp, resplen);
            free(resp);
            iprintf("[>] %d GET %s\n", status, request);
        } else {
            send_status(sockfd, NOT_FOUND);
            send_body(sockfd, NULL, 0);
            iprintf("[>] 404 GET %s\n", request);
        }
    } else if (strcmp(method, POST) == 0) {
        if (strcmp(request, "/") == 0) {
            send_status(sockfd, OK);
            send_header(sockfd, CONTENT_TYPE, "application/json");

            int firstNumber = 0;
            int secondNumber = 0;
            sscanf(body, "firstNumber=%d&secondNumber=%d", &firstNumber, &secondNumber);

            char* msg = (char*)malloc(sizeof(PAYLOAD) + 32);
            sprintf(msg, PAYLOAD, firstNumber + secondNumber);
            send_body(sockfd, msg, strlen(msg));
            free(msg);

            iprintf("[>] 200 POST %s\n", request);
        } else {
            send_status(sockfd, NOT_FOUND);
            send_body(sockfd, NULL, 0);
            iprintf("[>] 404 GET %s\n", request);
        }
    } else {
        send_status(sockfd, NOT_IMPLEMENTED);
        send_body(sockfd, NULL, 0);
        iprintf("[>] 501 %s %s\n", method, request);
    }
}

void send_status(int sockfd, int status) {
    char* msg = (char*)malloc(64);
    strcpy(msg, "HTTP/1.0 ");
    switch (status) {
        case OK:
            strcat(msg, "200 OK");
            break;
        case CONTINUE:
            strcat(msg, "201 Continue");
            break;
        case BAD_REQUEST:
            strcat(msg, "400 Bad Request");
            break;
        case NOT_FOUND:
            strcat(msg, "404 Not Found");
            break;
        case REQUEST_ENTITY_TOO_LARGE:
            strcat(msg, "431 Request Entity Too Large");
            break;
        case 0:
        case INTERNAL_SERVER_ERROR:
            strcat(msg, "500 Internal Server Error");
            break;
        case NOT_IMPLEMENTED:
            strcat(msg, "501 Not Implemented");
            break;
    }
    strcat(msg, "\r\n");
    send(sockfd, msg, strlen(msg), 0);
    send_header(sockfd, SERVER, "/nds/null v4.2.0");
    free(msg);
}
