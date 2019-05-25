#include <cstdlib> // C standard library header
#include <cstdio>
#include <sys/types.h>
#include <sys/socket.h> // socket header
#include <netdb.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <string>
#include <cstring>
#include <assert.h>
#include <errno.h>

#include "serverBase.h"
#include "rio.h"

#define IDLE 10
#define CNT 2
#define INTVL 5
#define MAXLINE 1024

ServerBase::ServerBase() {
    this->sfd = -1;
    this->cfd = -1;
}

void ServerBase::createServer(int port) {
    fprintf(stdout, "Starting Server!\n");
    assert(port > 0);

    struct sockaddr_in saddr;
    
    if ((this->sfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        return;
    }

    if (setServerSockOpts() < 0) { fprintf(stdout, "WHUT\n"); }

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

int ServerBase::setServerSockOpts() {
    int flags;

    // these if-statements setup the handshake to keep
    // alive the connection to client
    // if client does not respond, SIGPIPE is issued
    flags = 1; // enables address reuse
    if (setsockopt(this->sfd, SOL_SOCKET, SO_REUSEADDR,
                   (const void*)&flags, sizeof(flags)) < 0) {
        fprintf(stderr, "\t\t\t%s\n", strerror(errno));
        return -1;
    }

    flags = 1;
    if (setsockopt(this->sfd, SOL_SOCKET, SO_KEEPALIVE,
                   (const void*)&flags, sizeof(flags)) < 0) {
        fprintf(stderr, "\t\t\t%s\n", strerror(errno));
        return -1;
    }
    

    flags = IDLE; // heartbeat frequency
    if (setsockopt(this->sfd, IPPROTO_TCP, TCP_KEEPIDLE,
                   (const void*)&flags, sizeof(flags)) < 0) {
        fprintf(stderr, "\t\t\t%s\n", strerror(errno));
        return -1;
    }
    flags = CNT; // defines number of missed heartbeats as dropped client
    if (setsockopt(this->sfd, IPPROTO_TCP, TCP_KEEPCNT,
                   (const void*)&flags, sizeof(flags)) < 0) {
        fprintf(stderr, "\t\t\t%s\n", strerror(errno));
        return -1;
    }
    flags = INTVL; // heatbeat freq when client isn't responding
    if (setsockopt(this->sfd, IPPROTO_TCP, TCP_KEEPINTVL,
                   (const void*)&flags, sizeof(flags)) < 0) {
        fprintf(stderr, "\t\t\t%s\n", strerror(errno));
        return -1;
    }

    return 0;

}

int ServerBase::checkSockOpts() {
    int optval, optlen;

    int fail = 0;

    getsockopt(this->sfd, SOL_SOCKET, SO_REUSEADDR, &optval,(unsigned int*)&optlen);
    fprintf(stderr, "optval: %d\n", optval);
    if (optval != 1) {
        fprintf(stderr, "Error: SO_REUSEADDR not enabled!\n");
        fail = -1;
    }

    getsockopt(this->sfd, IPPROTO_TCP, TCP_KEEPIDLE, &optval, (unsigned int*)&optlen);
    fprintf(stderr, "optval: %d\n", optval);
    if (optval != IDLE) {
        fprintf(stderr, "Error: TCP_KEEPIDLE not set!\n");
        fail = -1;
    }

    getsockopt(this->sfd, IPPROTO_TCP, TCP_KEEPCNT, &optval, (unsigned int*)&optlen);
    fprintf(stderr, "optval: %d\n", optval);
    if (optval != CNT) {
        fprintf(stderr, "Error: TCP_KEEPCNT not set!\n");
        fail = -1;
    }

    getsockopt(this->sfd, IPPROTO_TCP, TCP_KEEPINTVL, &optval, (unsigned int*)&optlen);
    fprintf(stderr, "optval: %d\n", optval);
    if (optval != INTVL) {
        fprintf(stderr, "Error: TCP_KEEPINTVL not set!\n");
        fail = -1;
    }
   /* 
    getsockopt(this->cfd, SOL_SOCKET, SO_KEEPALIVE, &optval, (unsigned int*)&optlen);
    fprintf(stderr, "optval: %d\n", optval);
    if (optval != 1) {
        fprintf(stderr, "Error: SO_KEEPALIVE not enabled!\n");
        fail = -1;
    }*/


    return fail;
}

void ServerBase::run() {
    assert(this->sfd >= 0);
    pthread_t tid;

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

        if (pthread_create(&tid, NULL, thread, (void *)(long)this) < 0) {
            close(this->cfd);
            this->cfd = -1;
            continue;
        }
    }
}

int ServerBase::createClient() {
    struct sockaddr_in caddr;
    socklen_t caddr_size;

    caddr_size = sizeof(struct sockaddr_in);

    // acept() blocks until client connects
    assert(this->sfd >= 0);

    int rc = accept(this->sfd, (struct sockaddr *)&caddr.sin_addr.s_addr, &caddr_size);

    if (rc < 0) { return -1; }

    this->cfd = rc;
    return 0;
}

int ServerBase::setClientSockOpts() {
    int flags = 1;
    if (setsockopt(this->cfd, SOL_SOCKET, SO_KEEPALIVE,
                   (const void *)&flags, sizeof(flags)) < 0) {
        fprintf(stderr, "\t\t\t%s\n", strerror(errno));
        return -1;
    }
    return 0;

}

void* ServerBase::thread(void *arg) {
    fprintf(stdout, "BASE CLASS THREAD\n");
    assert(arg != NULL);

    pthread_detach(pthread_self());
    
   // char *c_ip = inet_ntoa(caddr.sin_addr);
   // std::string ip(c_ip);

    ServerBase *server = (ServerBase *)(long)arg;
    std::string msg = "Server: Connected\n";// + ip + "\n";
    server->sendToClient(msg.c_str());
    while (1) {
        server->setClientSockOpts();
        msg = "in loop\n";
        server->sendToClient(msg.c_str());
    }

    close(server->cfd);
    server->cfd = -1;
    return NULL;
}

int ServerBase::readFromClient(char *cmdline) {
    assert(cmdline != NULL);
    assert(this->cfd >= 0);
    
    int rc;
    if ((rc = read(this->cfd, cmdline, MAXLINE)) < 0) {
        if (errno == ECONNRESET) {
            fprintf(stdout, "%s\n", strerror(errno));
            return -1;
        } else if (errno == EPIPE) {
            fprintf(stdout, "%s\n", strerror(errno));
            return -1;
        } else if (errno == ETIMEDOUT) {
            fprintf(stdout, "%s\n", strerror(errno));
            return -1;
        } else { 
            fprintf(stdout, "%s\n", strerror(errno));
            return -1;
        }
    } else if (rc > 0) {
        return 1;
    }

    return 0;
}

void ServerBase::sendToClient(const char *msg) {
    assert(msg != NULL);
    assert(this->cfd >= 0);

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

ServerBase::~ServerBase() {

}

