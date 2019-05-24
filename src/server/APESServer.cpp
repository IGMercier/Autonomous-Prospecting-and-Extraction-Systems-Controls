#include "APESServer.h"
#include <cstdlib>
#include <cstdio>
#include <unistd.h>
#include <cstring>
#include <assert.h>
#include <errno.h>
#include <thread>
#include "../misc/flags.h"

static void sigpipe_handler(int sig);
static void connection(APESServer *server);

APESServer::APESServer(int *fd) {
    signal(SIGPIPE, sigpipe_handler);
    this->fd = fd;    
}

void APESServer::run() {
    assert(this->sfd >= 0);

    while (!shutdownSIG) {
        int val = createClient();
        if (val == -1) {
            this->cfd = -1;
            continue;
        } else if (val == -2) {
            close(this->cfd);
            this->cfd = -1;
            continue;
        }

        std::thread child(connection, this);
        child.detach();
    }
    shutdown();
    return; // kills server thread in main program
}

static void connection(APESServer *server) {
    assert(server != NULL);
    
    disconnected = 0;

    std::string msg = "Connected!\n";
    server->sendToClient(msg);
    fprintf(stdout, msg.c_str());

    while (!disconnected && !shutdownSIG) {
        server->setClientSockOpts();
        fprintf(stdout, "\t\t\t%d\n", disconnected);

        fd_mtx.lock();
        *(server->fd) = server->cfd;
        fd_mtx.unlock();
    }

    close(server->cfd);
    server->cfd = -1;

    fd_mtx.lock();
    *(server->fd) = -1;
    fd_mutex.unlock();
    fprintf(stdout, "Disconnected!\n");
    return;

}

void APESServer::shutdown() {
    std::string msg = "Server shutting down!\n";
    sendToClient(msg);
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

/* REMOVE AFTER TESTING */
static void sigpipe_handler(int sig) {
    int old_errno = errno;
    sigset_t mask, prev;
    sigprocmask(SIG_BLOCK, &mask, &prev);

    fprintf(stdout, "IN SIG HANDLER\n");

    disconnected = 1;

    sigprocmask(SIG_SETMASK, &prev, NULL);
    errno = old_errno;
    return;
}
