#include "APESServer.h"
#include <cstdlib>
#include <cstdio>
#include <unistd.h>
#include <cstring>
#include <assert.h>
#include <errno.h>
#include <thread>
#include "../misc/flags.h"
#include "../misc/flags_set.h"

static void connection(APESServer *server);

APESServer::APESServer(int *fd) {
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
    
    unsetDisconnected();

    std::string msg = "Connected!\n";
    server->sendToClient(msg.c_str());
    fprintf(stdout, msg.c_str());

    while (!disconnected && !shutdownSIG) {
        server->setClientSockOpts();
        *(server->fd) = server->cfd;
    }

    close(server->cfd);
    server->cfd = -1;
    *(server->fd) = -1;
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
