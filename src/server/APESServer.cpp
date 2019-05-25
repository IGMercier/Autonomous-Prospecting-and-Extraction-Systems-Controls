#include "APESServer.h"
#include <cstdlib>
#include <cstdio>
#include <csignal>
#include <unistd.h>
#include <cstring>
#include <assert.h>
#include <errno.h>
#include <netinet/tcp.h>

static volatile int disconnected = 1;

static void sigint_handler(int sig);
static void sigpipe_handler(int sig);

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
    disconnected = 0;

    std::string msg = "Connected!\n";
    sendToClient(msg.c_str());
    fprintf(stdout, msg.c_str());
    int flags;
    while (1) {
/*
        flags = 1;
        if (setsockopt(this->cfd, SOL_SOCKET, SO_KEEPALIVE,
                       (const void *)&flags, sizeof(flags)) < 0) {
            fprintf(stderr, "\t\t\t%s\n", strerror(errno));
            continue;
        }
*/

        char *cmdline = (char *)calloc(MAXLINE, sizeof(char));

        if (read(this->cfd, cmdline, MAXLINE) < 0) {//readFromClient(cmdline);
            if (errno == ECONNRESET) {
                fprintf(stdout, "%s\n", strerror(errno));
                free(cmdline);
                break;
            } else if (errno == EPIPE) {
                fprintf(stdout, "%s\n", strerror(errno));
                free(cmdline);
                break;
            } else if (errno == ETIMEDOUT) {
                fprintf(stdout, "%s\n", strerror(errno));
                free(cmdline);
                break;
            } else { 
                fprintf(stdout, "%s\n", strerror(errno));
                free(cmdline);
                break;
            }
        } else {
            fprintf(stdout, "Received: %s\n", cmdline);
        }
        free(cmdline);

    }

    close(this->cfd);
    this->cfd = -1;
    fprintf(stdout, "Disconnected!\n");
    return;

}

void APESServer::shutdown() {
    //robot.finish();

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

    //robot.shutdown();
    
    fprintf(stdout, "RECIEVED SIGINT\n");
    sigprocmask(SIG_SETMASK, &prev, NULL);
    errno = old_errno;

    _exit(0);
    return;
}

static void sigpipe_handler(int sig) {
    int old_errno = errno;

    sigset_t mask, prev;
    sigprocmask(SIG_BLOCK, &mask, &prev);
    
    disconnected = 1;

    fprintf(stdout, "RECIEVED SIGPIPE\n");
    sigprocmask(SIG_SETMASK, &prev, NULL);
    errno = old_errno;
    return;
}
