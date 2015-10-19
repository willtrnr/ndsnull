#ifdef ARM9
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

int main(void) {
#ifdef ARM9
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
#else
    return http_server(HTTP_PORT);
#endif
}
