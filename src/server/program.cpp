#include <cstdlib>
#include <cstdio>
#include "APESServer.h"

int main(int argc, char **argv) {

    int port;
    if (argc < 2) {
        port = 16778;
    } else {
        port = atoi(argv[1]);
    }

    APESServer server = APESServer("cmd.txt", "log.txt");

    while (server.sfd < 0) {
        server.createServer(port);
    }
    //server.setSockOpts();
    server.run();

    fprintf(stdout, "OOPS\n");
    server.shutdown();

    return -1;
}

