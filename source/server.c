#include "server.h"

int http_server(int port) {
    int servfd = socket(PF_INET, SOCK_STREAM, 0);
    if (servfd < 0) {
        iprintf("[!] Could not create socket!");
        return 1;
    }

    if (setsockopt(servfd, SOL_SOCKET, SO_REUSEADDR, &(int){ 1 }, sizeof(int)) < 0) {
        iprintf("[!] Could not set reuse option!");
    }

    if (setsockopt(servfd, SOL_TCP, TCP_NODELAY, &(int){ 1 }, sizeof(int)) < 0) {
        iprintf("[!] Could not set nodelay option!");
    }

    #ifdef TCP_CORK
        if (setsockopt(servfd, SOL_TCP, TCP_CORK, &(int){ 1 }, sizeof(int)) < 0) {
            iprintf("[!] Could not set cork option!");
        }
    #endif

    #ifdef TCP_DEFER_ACCEPT
        if (setsockopt(servfd, SOL_TCP, TCP_DEFER_ACCEPT, &(int){ 1 }, sizeof(int)) < 0) {
            iprintf("[!] Could not set defer option!");
        }
    #endif

    #ifdef TCP_QUICKACK
        if (setsockopt(servfd, SOL_TCP, TCP_QUICKACK, &(int){ 1 }, sizeof(int)) < 0) {
            iprintf("[!] Could not set cork option!");
        }
    #endif

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(servfd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        closesocket(servfd);
        iprintf("[!] Could not bind to port %d!", port);
        return 1;
    }

    if (listen(servfd, 10) < 0) {
        closesocket(servfd);
        iprintf("[!] Could not listen on socket!");
        return 1;
    }

    iprintf("[*] Listening on %s:%d...\n", inet_ntoa(addr.sin_addr), port);
    for (;;) accept_client(servfd);
}

void accept_client(int servfd) {
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    int addrlen = 0;
    int sockfd = accept(servfd, (struct sockaddr*)&addr, &addrlen);
    if (sockfd < 0) {
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
        } else {
            send_status(sockfd, NOT_FOUND);
            send_body(sockfd, NULL, 0);
            iprintf("[>] %d GET %s\n", NOT_FOUND, request);
        }
    } else if (strcmp(method, POST) == 0) {
        if (strcmp(request, "/") == 0) {
            char* buf = (char*)malloc(bodylen);

            const char* p;
            for (p = body; *p; ++p) buf[p - body] = *p > 0x40 && *p < 0x5b ? *p | 0x60 : *p;
            buf[bodylen] = 0;

            char* resp = NULL;
            int resplen = 0;
            int status = http_request(POST, "127.0.0.1", "/challenge", buf, bodylen, &resp, &resplen);

            buf = (char*)realloc(buf, sizeof(PAYLOAD) + resplen);
            sprintf(buf, PAYLOAD, resp);
            send_status(sockfd, OK);
            send_body(sockfd, buf, strlen(buf));

            free(resp);
            free(buf);
            iprintf("[>] %d POST %s\n", OK, request);
        } else if (strcmp(request, "/challenge") == 0) {
            json_value* val = json_parse(body, bodylen);
            json_value* arr = json_array_new(0);
            int i;
            for (i = 0; i < val->u.object.values[1].value->u.object.length; ++i) {
                int p = 0;
                sscanf(val->u.object.values[1].value->u.object.values[i].name, "%d", &p);
                if (strstr(val->u.object.values[1].value->u.object.values[i].value->u.string.ptr, val->u.object.values[0].value->u.string.ptr) != NULL) {
                    json_array_push(arr, json_integer_new(p));
                }
            }
            json_value_free(val);

            char* buf = malloc(json_measure(arr));
            json_serialize(buf, arr);
            json_builder_free(arr);

            send_status(sockfd, OK);
            send_body(sockfd, buf, strlen(buf));
            iprintf("[>] %d POST %s\n", OK, request);
        } else {
            send_status(sockfd, NOT_FOUND);
            send_body(sockfd, NULL, 0);
            iprintf("[>] %d POST %s\n", NOT_FOUND, request);
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
