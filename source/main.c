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
#endif
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
#ifdef ARM9
    }
#endif

    return 0;
}
