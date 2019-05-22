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
    //server.setSockOpts();
    server.run();

    fprintf(stdout, "OOPS\n");
    server.shutdown();

    return -1;
}

