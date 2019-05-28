#include <thread>
#include <string>
#include "../server/APESServer.h"

static void serverThread(sysArgs *args, int port);

int main(int argc, char **argv) {

    //sig_atomic_t disconnected = 1;
    //sig_atomic_t shutdownSIG = 0;

    int port;
    if (argc < 2) {
        port = 16778;
    } else {
        port = atoi(argv[1]);
    }

    std::mutex cmd_mtx;
    std::mutex log_mtx;

    std::deque<char *> *cmdq = new std::deque<char *>;
    std::deque<char *> *logq = new std::deque<char *>;

    std::string msg = "System started!";

    logq->push_back((char *)msg.c_str());
    msg = "System standby!";
    logq->push_back((char *)msg.c_str());
    msg = "System stop!";
    logq->push_back((char *)msg.c_str());

    sysArgs args;
    args.cmd_mtx = &cmd_mtx;
    args.log_mtx = &log_mtx;
    args.cmdq = cmdq;
    args.logq = logq;

    std::thread t(serverThread, &args, port);
    t.join();

    return -1;
}

static void serverThread(sysArgs *args, int port) {
    APESServer *server = new APESServer(args);
    
    while (server->sfd < 0) {
        server->createServer(port);
    }
    server->run();

    // control flow should never reach
    //server->shutdown();
}
