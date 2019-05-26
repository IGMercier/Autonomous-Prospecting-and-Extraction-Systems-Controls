#include <cstdlib>
#include <cstdio>
#include <csignal>
#include <errno.h>
#include <thread>
#include "../server/APESServer.h"
//#include "../misc/flags.h"

int main(int argc, char **argv) {

    //sig_atomic_t disconnected = 1;
    //sig_atomic_t shutdownSIG = 0;

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
    server.run();

    // control flow should never reach
    server.shutdown();
    return -1;
}
