#include "../server/APESServer.h"

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

    logq->push_back("Sytem started!");
    logq->push_back("Sytem standby!");
    logq->push_back("Sytem shutting down!");
    logq->push_back("Sytem drive!");
    logq->push_back("Sytem stop!");

    sysArgs args;
    args.cmd_mtx = &cmd_mtx;
    args.log_mtx = &log_mtx;
    args.cmdq = cmdq;
    args.logq = logq;


    APESServer *server = new APESServer(&args);
    
    while (server->sfd < 0) {
        server->createServer(port);
    }
    server->run();

    // control flow should never reach
    server->shutdown();
    return -1;
}
