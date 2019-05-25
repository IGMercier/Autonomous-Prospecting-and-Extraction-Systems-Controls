#include "APESServer.h"
#include <cstdlib>
#include <cstdio>
#include <csignal>
#include <unistd.h>
#include <cstring>
#include <assert.h>
#include <errno.h>
#include <netinet/tcp.h>

static void sigint_handler(int sig);

APESServer::APESServer() {
    signal(SIGINT, sigint_handler);
    signal(SIGPIPE, SIG_IGN);
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
    
    char *cmdline = (char *)calloc(MAXLINE, sizeof(char));
    while (1) {
        setClientSockOpts();
        setServerSockOpts();
        //checkSockOpts();

        memset(cmdline, 0, MAXLINE);

        int rc;
        if ((rc = read(this->cfd, cmdline, MAXLINE)) < 0) {//readFromClient(cmdline);
            if (errno == ECONNRESET) {
                fprintf(stdout, "%s\n", strerror(errno));
                break;
            } else if (errno == EPIPE) {
                fprintf(stdout, "%s\n", strerror(errno));
                break;
            } else if (errno == ETIMEDOUT) {
                fprintf(stdout, "%s\n", strerror(errno));
                break;
            } else { 
                fprintf(stdout, "%s\n", strerror(errno));
                break;
            }
        } else if (rc > 0) {
            cmdline[strlen(cmdline)-1] = '\0';
            fprintf(stdout, "Received: %s\n", cmdline);
        }

    }

    free(cmdline);
    close(this->cfd);
    this->cfd = -1;
    fprintf(stdout, "Disconnected!\n");
    return;

}

void APESServer::shutdown() {

    std::string msg = "System shutting down!\n";
    sendToClient(msg.c_str());
    fprintf(stdout, "%s", msg.c_str());

    msg = "Server shutting down!\n";
    sendToClient(msg.c_str());
    fprintf(stdout, "%s", msg.c_str());

    if (close(this->cfd) < 0) {
        fprintf(stderr, "ERROR: %s\n", strerror(errno));
    }
    if (close(this->sfd) < 0) {
        fprintf(stderr, "ERROR: %s\n", strerror(errno));
    }

    exit(0);
}

APESServer::~APESServer() {
}

static void sigint_handler(int sig) {
    int old_errno = errno;

    sigset_t mask, prev;
    sigprocmask(SIG_BLOCK, &mask, &prev);

    
    fprintf(stdout, "RECIEVED SIGINT\n");
    sigprocmask(SIG_SETMASK, &prev, NULL);
    errno = old_errno;

    _exit(0);
    return;
}
