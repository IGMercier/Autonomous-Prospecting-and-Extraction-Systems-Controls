#include <cstdlib>
#include <cstdio>
#include <csignal>
#include <errno.h>
#include "../server/APESServer.h"
#include "../misc/flags.h"


int main(int argc, char **argv) {

    disconnected = 1;
    shutdownSIG = 0;

    int port;
    if (argc < 2) {
        port = 16778;
    } else {
        port = atoi(argv[1]);
    }

    APESServer server = APESServer(NULL, NULL);
    while (server.sfd < 0) {
        server.createServer(port);
    }
    server.run();

    // control flow should never reach
    server.shutdown();

    return -1;
}


