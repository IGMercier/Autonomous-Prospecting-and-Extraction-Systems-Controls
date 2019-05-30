#include <cstdlib>
#include <cstdio>
#include <csignal>
#include <fstream>
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
static int shutdown_sig = 0;

APESServer::APESServer(sysArgs *args) {
    signal(SIGPIPE, SIG_IGN);
    assert(args != NULL);
    assert(args->cmd_mtx != NULL);
    assert(args->log_mtx != NULL);
    assert(args->data_mtx != NULL);
    assert(args->cmdq != NULL);
    assert(args->logq != NULL);
    assert(!args->datafile.empty());

    this->cmd_mtx = args->cmd_mtx;
    this->log_mtx = args->log_mtx;
    this->data_mtx = args->data_mtx;

    this->cmdq = args->cmdq;
    this->logq = args->logq;

    this->datafile = datafile;
}

void APESServer::run(int port) {
    while (this->sfd < 0) {
        createServer(port);
    }
    assert(this->sfd >= 0);

    while (!shutdown_sig) {
        createClient();

        assert(this->cfd >= 0);
        execute();
    }

    shutdown();
    return;
}

void APESServer::execute() {
    std::string msg = "Connected!\n";
    if (sendToClient(msg.c_str()) < 0) {
        disconnected();
        return;
    }

    if (sendToClient(delim) < 0) {
        disconnected();
        return;
    }
    
    char *cmdline = new char[MAXLINE];
    while (1) {
        setClientSockOpts();
        setServerSockOpts();

        memset(cmdline, 0, MAXLINE);

        int rc;
        while ((rc = readFromClient(cmdline)) > 0) {
            //print(cmdline);

            if (!strncmp(cmdline, delim, MAXLINE)) {
                break;
            } else {
                cmdline[strlen(cmdline)-1] = '\0';

                // remove next 4 lines in real system!
                //std::unique_lock<std::mutex> loglock(*(this->log_mtx));
                //std::string buf_ = cmdline;
                //this->logq->push_back(buf_);
                //loglock.unlock();

                // writes to command file for shell to read
                std::unique_lock<std::mutex> cmdlock(*(this->cmd_mtx));
                std::string buf = cmdline;
                this->cmdq->push_back(buf);
                cmdlock.unlock();
            }
        }

        if (rc < 0) { break; }

        // reads off whatever the shell has logged
        std::unique_lock<std::mutex> loglock(*(this->log_mtx));
        while (!this->logq->empty()) {
            std::string logline = this->logq->at(0);
            printf("%s", logline.c_str());

            if (logline == shutdown_tag) {
                shutdown_sig = 1;
                rc = -1;
                break;
            }

            if ((rc = sendToClient(logline.c_str()) < 0)) {
                break;
            }
            
            if ((rc = sendToClient(delim)) < 0) { break; }

            this->logq->pop_front();;
        }
        loglock.unlock();

        if (rc < 0) { break; }

        /*
        // reads off data file
        std::unique_lock<std::mutex> datalock(*(this->data_mtx));
        {
            std::ifstream data(this->datafile);
            if (data) {
                std::string dataline;
                while (getline(data, dataline)) {
                    sendToClient(dataline.c_str());
                }
            }
        }
        datalock.unlock();
        */

    }

    delete cmdline;

    if (!shutdown_sig) {
        disconnected();
    }

    return;

}

void APESServer::disconnected() {
    // if disconnected, clears command queue and inserts standby command
    std::unique_lock<std::mutex> cmdlock(*(this->cmd_mtx));
    this->cmdq->clear();
    std::string buf = "standby";
    this->cmdq->push_back(buf);
    cmdlock.unlock();
    
    close(this->cfd);
    this->cfd = -1;
    print("Disconnected!");

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
    exit(0);
}

APESServer::~APESServer() {
}

