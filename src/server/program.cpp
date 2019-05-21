#include <cstdlib>
#include <cstdio>
#include "server.h"

int main(int argc, char **argv) {

    int port;
    if (argc < 2) {
        port = 16778;
    } else {
        port = atoi(argv[1]);
    }

    Server server = Server();
    while (server.sfd < 0) {
        server.createServer(port);
    }
    server.run();

    // control flow should never reach
    server.shutdown();

    return -1;
}
