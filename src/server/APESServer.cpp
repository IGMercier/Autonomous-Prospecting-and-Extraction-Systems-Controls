#include <cstdlib>
#include <cstdio>
#include <csignal>
#include <unistd.h>
#include <cstring>
#include <assert.h>
#include <errno.h>
#include <netinet/tcp.h>

#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <arpa/inet.h>

#include "APESServer.h"

static char *delim = "\r\n";

APESServer::APESServer(sysArgs *args) {
    signal(SIGPIPE, SIG_IGN);
    assert(args != NULL);
    assert(args->cmd_mtx != NULL);
    assert(args->log_mtx != NULL);
    assert(args->cmdq != NULL);
    assert(args->logq != NULL);

    this->cmd_mtx = args->cmd_mtx;
    this->log_mtx = args->log_mtx;
    this->cmdq = args->cmdq;
    this->logq = args->logq;
}

void APESServer::run(int port) {
    while (this->sfd < 0) {
        createServer(port);
    }
    assert(this->sfd >= 0);

    while (1) {
        createClient();
        if (this->cfd < 0);
        
        //assert(this->cfd >= 0);
        execute();
    }
}

void APESServer::execute() {
    std::string msg = "Connected!\n";
    sendToClient(msg.c_str());
    //msg = "END";
    //sendToClient(msg.c_str());
    
    char *cmdline = (char *)calloc(MAXLINE, sizeof(char));
    while (1) {
        setClientSockOpts();
        setServerSockOpts();
        //checkSockOpts();

        memset(cmdline, 0, MAXLINE);

        int rc;
        while ((rc = readFromClient(cmdline)) > 0) {
                //cmdline[strlen(cmdline)-1] = '\0';
               
                if (!strncmp(cmdline, delim, MAXLINE)) {
                    break;
                } else {
                    // writes to command file for shell to read
                    std::unique_lock<std::mutex> cmdlock(*(this->cmd_mtx));
                    std::string buf = cmdline;
                    this->cmdq->push_back(buf);
                    cmdlock.unlock();
                }
            }
        }

        if (rc < 0) { break; }

        std::unique_lock<std::mutex> loglock(*(this->log_mtx));
        while (!this->logq->empty()) {
            std::string logline = this->logq->at(0);
            printf("%s", logline.c_str());
            sendToClient(logline.c_str());
            this->logq->pop_front();;
        }
        loglock.unlock();
        sendToClient(delim);
    }

    free(cmdline);

    // if disconnected, clears command queue and inserts standby command
    std::unique_lock<std::mutex> cmdlock(*(this->cmd_mtx));
    this->cmdq->clear();
    std::string buf = "standby";
    this->cmdq->push_back(buf);
    cmdlock.unlock();
    
    close(this->cfd);
    this->cfd = -1;
    print("Disconnected!");
    return;

}

void APESServer::shutdown() {
    std::string msg = "Server shutting down!\n";
    sendToClient(msg.c_str());
    print(msg.c_str());

    if (this->cfd >= 0) {
        if (close(this->cfd) < 0) {
            print(strerror(errno));
        }
    }
    if (this->sfd >= 0) {
        if (close(this->sfd) < 0) {
            print(strerror(errno));
        }
    }
    return;
}

APESServer::~APESServer() {
}

