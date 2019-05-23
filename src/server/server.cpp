#include "server.h"
#include <cstdlib>
#include <cstdio>
#include <unistd.h>
#include <cstring>
#include <assert.h>
#include <errno.h>
#include <thread>
#include "../flags.h" // disconnected

static void connection(Server *server, int *shell_cfd);

Server::Server() {}

void Server::run(int *shell_cfd) {
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

        std::thread child(connection, this, shell_cfd);
        child.detach();
    }
    shutdown();
    return; // kills server thread in main program
}

static void connection(Server *server, int *shell_cfd) {
    assert(server != NULL);
    
    disconnected = 0;

    std::string msg = "Connected!\n";
    server->sendToClient(msg.c_str());
    fprintf(stdout, msg.c_str());

    while (!disconnected) {
        server->setClientSockOpts();
        *shell_cfd = server->cfd;
    }

    close(server->cfd);
    server->cfd = -1;
    fprintf(stdout, "Disconnected!\n");
    return;

}

void Server::shutdown() {
    std::string msg = "Server shutting down!\n";
    sendToClient(msg.c_str());
    fprintf(stdout, "%s", msg.c_str());

    if (close(this->cfd) < 0) {
        fprintf(stderr, "ERROR: %s\n", strerror(errno));
    }
    if (close(this->sfd) < 0) {
        fprintf(stderr, "ERROR: %s\n", strerror(errno));
    }
    return;
}

Server::~Server() {
}
