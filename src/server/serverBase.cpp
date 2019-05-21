#include <cstdlib> // C standard library header
#include <sys/socket.h> // socket header
#include <pthread.h>
#include <string>
#include <assert>

#include "serverBase.h"
#include "rio.h"

ServerBase::ServerBase() {
    this->sfd = -1;
    this->cfd = -1;
}

void ServerBase::serverSetup(int port) {
    assert(port > 0);

    int flags;
    struct sockaddr_in saddr;
    
    if ((this->sfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        return;
    }

    // these if-statements setup the handshake to keep
    // alive the connection to client
    // if client does not respond, SIGPIPE is issued
    flags = 1; // enables address reuse
    if (setsockopt(this->sfd, SOL_SOCKET, SO_REUSEADDR,
                   (const void*)&flags, sizeof(int)) < 0) {
        close(this->sfd);
        this->sfd = -1;
        return;
    }

    flags = 10; // heartbeat frequency
    if (setsockopt(this->sfd, SOL_TCP, TCP_KEEPIDLE,
                   (const void*)&flags, sizeof(int)) < 0) {
        close(this->sfd);
        this->sfd = -1;
        return;
    }
    flags = 2; // defines number of missed heartbeats as dropped client
    if (setsockopt(this->sfd, SOL_TCP, TCP_KEEPCNT,
                   (const void*)&flags, sizeof(int)) < 0) {
        close(this->sfd);
        this->sfd = -1;
        return;
    }
    flags = 2; // heatbeat freq when client isn't responding
    if (setsockopt(this->sfd, SOL_TCP, TCP_KEEPINTVL,
                   (const void*)&flags, sizeof(int)) < 0) {
        close(this->sfd);
        this->sfd = -1;
        return;
    }

    memset(&saddr, 0, sizeof(saddr));
    saddr.sin_family = AF_INET;
    saddr.sin_addr.s_addr = htonl(INADDR_ANY);
    saddr.sin_port = htons((unsigned short)port);
    
    if ((bind(this->sfd,
             (struct sockaddr *)&saddr,
             sizeof(saddr))) < 0) {
        close(this->sfd);
        this->sfd = -1;
        return;
    }

    // allows only 1 connection before it starts rejecting connections
    if (listen(this->sfd, 1) < 0) {
        close(this->sfd);
        this->sfd = -1;
        return;
    }
    return;
}

void ServerBase::clientSetup() {
    assert(this->sfd >= 0);

    pthread_t tid;
    int flags = 1;

    while (1) {
        assert(this->cfd < 0);

        struct sockaddr_in caddr;
        socklen_t caddr_size;

        caddr_size = sizeof(struct sockaddr_in);

        // acept() blocks until client connects
        if ((this->cfd = accept(this->sfd,
                                (struct sockaddr *)&caddr.sin_addr.s_addr,
                                &caddr_size)) < 0) {
            this->cfd = -1;
            continue;
        }

        flags = 1;
        if (setsockopt(this->cfd, SOL_SOCKET, SO_KEEPALIVE,
                       (const void *)$flags, sizeof(int)) < 0) {
            close(this->cfd);
            this->cfd = -1;
            continue;
        }

        std::string msg = "Connected...!\n";
        sendToClient(msg.c_str());

        if (pthread_create(&tid, NULL, thread, NULL) < 0) {
            close(this->cfd);
            this->cfd = -1;
            continue;
        }
    }

    return;
}

void* ServerBase::thread(void *arg) {
    pthread_detach(pthread_self());

    close(this->cfd);
    this->cfd = -1;
    return NULL;
}

void ServerBase::readFromClient(char *cmdline) {
    assert(cmdline != NULL);

    rio_t buf;
    rio_initb(&buf, this->cfd);
    rio_readlineb(&buf, cmdline, MAXLINE);

    cmdline[strlen(cmdline)-1] = '\0';

    return;
}

void ServerBase::sendToClient(const char *msg) {
    assert(msg != NULL);

    rio_writen(this->cfd, (void *)msg, strlen(msg));

    return;
}

void ServerBase::shutdown() {
    close(this->cfd);
    this->cfd = -1;
    close(this->sfd);
    this->sfd = -1;

    return;
}

ServerBase::~serverBase() {

}
