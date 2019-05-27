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
        struct sockaddr_in caddr;
        socklen_t caddr_size;

        caddr_size = sizeof(struct sockaddr_in);

        assert(this->sfd >= 0);
        // accept() blocks until client connects
        int rc = accept(this->sfd,
                        (struct sockaddr *)&caddr.sin_addr.s_addr,
                        &caddr_size);
        if (rc < 0) { continue; }
        this->cfd = rc;
        
        assert(this->cfd >= 0);
        execute();
    }
}

void APESServer::execute() {
    std::string msg = "Connected!\n";
    sendToClient(msg.c_str());
    fprintf(stdout,"%s",  msg.c_str());
    msg = "END";
    sendToClient(msg.c_str());
    
    char *cmdline = (char *)calloc(MAXLINE, sizeof(char));
    while (1) {
        setClientSockOpts();
        setServerSockOpts();
        //checkSockOpts();

        memset(cmdline, 0, MAXLINE);

        int fail = 0;
        std::unique_lock<std::mutex> cmdlock(*(this->cmd_mtx));
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
                    std::string buf(cmdline);
                    this->cmdq->push_back(buf);
                }

            } else if (rc < 0) { fail = 1; break; }
        }
        cmdlock.unlock();

        if (fail) { break; }


        std::unique_lock<std::mutex> loglock(*(this->log_mtx));
        while (!this->logq->empty()) {
            std::string logline = this->logq->at(0);
            printf("%s\n", logline.c_str());
            this->logq->pop_front();
        }
        fflush(stdout);
        loglock.unlock();
    }

    free(cmdline);

    // if disconnected, clears command queue and inserts standby command
    std::unique_lock<std::mutex> cmdlock(*(this->cmd_mtx));
    char *buf = (char *)calloc(MAXLINE, sizeof(char));
    strncpy(buf, "standby", MAXLINE);
    this->cmdq->clear();
    this->cmdq->push_back(buf);
    cmdlock.unlock();


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

