#include <cstdlib>
#include <cstdio>
#include <csignal>
#include <errno.h>
#include <thread>
#include "../server/APESServer.h"
//#include "../misc/flags.h"


static void serverThread(int port);

int main(int argc, char **argv) {

    //sig_atomic_t disconnected = 1;
    //sig_atomic_t shutdownSIG = 0;

    int port;
    if (argc < 2) {
        port = 16778;
    } else {
        port = atoi(argv[1]);
    }

    serverThread(port);
    //std::thread t(serverThread, port);
    //t.join();


    return -1;
}

static void serverThread(int port) {
    APESServer server = APESServer(NULL, NULL);
    while (server.sfd < 0) {
        server.createServer(port);
    }
    server.run();

    // control flow should never reach
    server.shutdown();
    return;
}

