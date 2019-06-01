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
    std::mutex data_mtx;

    std::deque<std::string> *cmdq = new std::deque<std::string>;
    std::deque<std::string> *logq = new std::deque<std::string>;
    cmdq->clear();
    logq->clear();

    logq->push_back("Server: heyo\n");

    sysArgs *args = new sysArgs;
    args->cmd_mtx = &cmd_mtx;
    args->log_mtx = &log_mtx;
    args->data_mtx = &data_mtx;
    args->cmdq = cmdq;
    args->logq = logq;
    args->datafile = "data.csv";

    std::thread t(serverThread, args, port);
    if (t.joinable()) {t.join(); }

    delete cmdq;
    delete logq;
    delete args;

    return -1;
}

static void serverThread(sysArgs *args, int port) {
    APESServer *server = new APESServer(args);
    
    server->run(port);

    // control flow should never reach here
    delete server;
    return;
}
