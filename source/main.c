#ifdef NDS
    #include <nds.h>
    #include <dswifi9.h>
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
#include "server.h"
#include "client.h"

#ifdef NDS
    int main(void) {
        consoleDemoInit();

        iprintf("/nds/null Web Server v4.2.0\n");
        iprintf("--------------------------------");

        iprintf("[*] Connecting...\n");
        if (!Wifi_InitDefault(WFC_CONNECT)) {
            iprintf("[!] Failed to connect!");
            return 1;
        } else {
            iprintf("[*] Connected!\n");

            return http_server(HTTP_PORT);
        }
    }
#else
    int main(int argc, char** argv) {
        int port = HTTP_PORT;
        if (argc > 1) {
            sscanf(argv[argc - 1], "%d", &port);
        }
        return http_server(port);
    }
#endif
