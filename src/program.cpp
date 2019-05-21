#include "server/server.h"

int main(int argc, char** argv) {
    int port;
    if (argc < 2) {
        port = 16778;
    } else {
        port = atoi(argv[1]);
    }

    Server server = Server();

    while (server.sfd < 0) {
        server.serverSetup(port);
    }

    assert(server.sfd >= 0);
    server.clientSetup();

    // control should never reach here
    server.shutdown();
    return -1;

}
