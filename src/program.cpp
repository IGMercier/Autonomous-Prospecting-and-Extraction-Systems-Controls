#include <thread>
#include <errno.h>

#include "server/APESServer.h"
#include "shell/APESShell.h"

static void serverThread(APESServer *server, int port);
static void serverWriteThread(APESServer *server);
static void shellThread(APESShell *shell);

std::string shutdown_tag = "<shutdown>";
std::string data_tag = "<data>";

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

    APESServer *server = new APESServer(args);
    APESShell *shell = new APESShell(args);
    
    std::thread tServer(serverThread, server, port);
    std::thread tServerWrite(serverWriteThread, server);
    std::thread tShell(shellThread, shell);


    if (tServer.joinable()) {
        tServer.join();
    }
    if (tServerWrite.joinable()) {
        tServer.join();
    }
    if (tShell.joinable()) {
        tShell.join();
    }

    delete server;
    delete shell;
    delete cmdq;
    delete logq;
    delete args;

    // control should never reach here
    return -1;

}

/*
    THREAD CALLBACKS
*/
static void serverThread(APESServer *server, int port) {
    assert(args != NULL);
        
    server->run(port);
	        
    // control should never reach here
    return;
}

static void serverWriteThread(APESServer *server) {
    assert(args != NULL);
        
    server->write();
	        
    // control should never reach here
    return;
}

		
static void shellThread(APESShell *shell) {
    assert(args != NULL);

    shell->run();

    // control should never reach here
    return;
}
