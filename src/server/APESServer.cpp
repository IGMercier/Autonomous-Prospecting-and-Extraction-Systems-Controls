#include "APESServer.h"
#include <cstdlib>
#include <cstdio>
#include <csignal>
#include <unistd.h>
#include <cstring>
#include <assert.h>
#include <errno.h>
#include <netinet/tcp.h>
#include <mutex>

#define MAXLINE 1024

static void sigint_handler(int sig);
std::mutex cmd_mtx;
std::mutex log_mtx;

APESServer::APESServer(std::deque<char *> *cmdq, std::deque<char *> *logq) {
    signal(SIGINT, sigint_handler);
    signal(SIGPIPE, SIG_IGN);

    this->cmdq = cmdq;
    this->logq = logq;
}

void APESServer::run() {
    assert(this->sfd >= 0);

    while (1) {
        int val = createClient();
        if (val == -1) {
            this->cfd = -1;
            continue;
        } else if (val == -2) {
            close(this->cfd);
            this->cfd = -1;
            continue;
        }
        execute();
    }
}

void APESServer::execute() {
    std::string msg = "Connected!\n";
    sendToClient(msg.c_str());
    fprintf(stdout, msg.c_str());
    msg = "END";
    sendToClient(msg.c_str());
    
    char cmdline[MAXLINE];
    while (1) {
        setClientSockOpts();
        setServerSockOpts();
        //checkSockOpts();

        memset(cmdline, 0, MAXLINE);

        int fail = 0;
        std::unique_lock<std::mutex> cmdlock(cmd_mtx);
        while (1) {
            int rc;
            if ((rc = readFromClient(cmdline)) > 0) {
                cmdline[strlen(cmdline)-1] = '\0';
                msg = "Received!\n";
                sendToClient(msg.c_str());
                //fprintf(stdout, "%s", msg.c_str());
                msg = "END";
                sendToClient(msg.c_str());
               
                if (!strncmp(cmdline, "END", MAXLINE)) {
                    break;
                } else {
                    // writes to command file for shell to read
                    char *buf = (char *)calloc(MAXLINE, sizeof(char));
                    strncpy(buf, cmdline, MAXLINE);
                    this->cmdq->push_back(buf);
                }

            } else if (rc < 0) { fail = 1; break; }
        }
        cmdlock.unlock();

        if (fail) { break; }


        std::unique_lock<std::mutex> loglock(log_mtx);
        if (!this->logq->empty()) {
            char *logline = this->logq->at(0);
            this->logq->pop_front();
            //printf("%s", logline);
        }
        loglock.unlock();
    }

    close(this->cfd);
    this->cfd = -1;
    fprintf(stdout, "Disconnected!\n");
    return;

}

void APESServer::shutdown() {
    std::string msg = "Server shutting down!\n";
    sendToClient(msg.c_str());
    fprintf(stdout, "%s", msg.c_str());

    if (this->cfd >= 0) {
        if (close(this->cfd) < 0) {
            fprintf(stderr, "ERROR: %s\n", strerror(errno));
        }
    }
    if (this->sfd >= 0) {
        if (close(this->sfd) < 0) {
            fprintf(stderr, "ERROR: %s\n", strerror(errno));
        }
    }
    return;
}

APESServer::~APESServer() {
}

static void sigint_handler(int sig) {
    _exit(0);
    return;
}
