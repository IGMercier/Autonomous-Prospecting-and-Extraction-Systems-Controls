#include <thread>
#include <errno.h>

#include "server/APESServer.h"
#include "shell/APESShell.h"

static void sigpipe_handler(int sig);
static void sigint_handler(int sig);

static void serverThread(sysArgs *args);
static void shellThread(sysArgs *args);

int main(int argc, char** argv) {
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
    cmdq->clear();
    logq->clear();

    sysArgs args;
    args.cmd_mtx = &cmd_mtx;
    args.log_mtx = &cmd_mtx;
    args.cmdq = cmdq;
    args.logq = logq;


    std::thread tServer(serverThread, &args);
    std::thread tShell(shellThread, &args);

    if (tServer.joinable()) {
        tServer.join();
    }

    if (tShell.joinable()) {
        tShell.join();
    }

    delete cmdq;
    delete logq;

    // control should never reach here
    return -1;

}


/*
    THREAD CALLBACKS
*/
static void serverThread(sysArgs *args) {
    assert(args != NULL);
    
    APESServer server = APESServer(args);
    while (server.sfd < 0) {
        server.serverSetup(port);
    }
    assert(server.sfd >= 0);
    server.run();
    
    // control should never reach here
    return;
}

   
static void shellThread(sysArgs *args) {
    assert(args != NULL);

    APESShell shell = APESShell(args);
    shell.run();

    // control should never reach here
    return;
}
