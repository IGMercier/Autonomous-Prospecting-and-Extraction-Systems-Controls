#include <thread>
#include <errno.h>
#include <assert.h>

#include "server/APESServer.h"
#include "shell/APESShell.h"

static void serverThread(sysArgs *args, int port);
static void shellThread(sysArgs *args);

std::string shutdown_tag = "<shutdown>";

int main(int argc, char** argv) {
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

    sysArgs *args = new sysArgs;
    args->cmd_mtx = &cmd_mtx;
    args->log_mtx = &log_mtx;
    args->data_mtx = &data_mtx;
    args->cmdq = cmdq;
    args->logq = logq;
    args->datafile = "data.csv";

    std::thread tServer(serverThread, args, port);
    std::thread tShell(shellThread, args);


    if (tServer.joinable()) {
        tServer.join();
    }
    if (tShell.joinable()) {
        tShell.join();
    }

    delete cmdq;
    delete logq;
    delete args;

    // control should never reach here
    return -1;

}


/*
    THREAD CALLBACKS
*/
static void serverThread(sysArgs *args, int port) {
    assert(args != NULL);
    
    APESServer *server = new APESServer(args);
    server->run(port);
    
    // control should never reach here
    delete server;
    return;
}

static void shellThread(sysArgs *args) {
    assert(args != NULL);

    APESShell *shell = new APESShell(args);
    shell->run();

    // control should never reach here
    delete shell;
    return;
}
