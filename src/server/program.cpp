#include <cstdlib>
#include "server.h"

int main(int argc, char **argv) {

    int port;
    if (argc < 2) {
        port = 16778;
    } else {
        port = atoi(argv[1]);
    }

    Server server = Server();
    server.serverSetup(port);
    server.clientSetup();

    server.shutdown();

    return -1;
}
