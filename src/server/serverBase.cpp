#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <csignal>
#include <sys/socket.h>
#include <sys/types.h> // for AF_INET
#include <netdb.h> // for ip_ntoa
#include <arpa/inet.h> // for ip_ntoa
#include <thread>
#include <netinet/tcp.h>
#include <assert.h>
#include <errno.h>
#include <sys/time.h>

#include "serverBase.h"
#include "../misc/rio.h"

using std::thread;

#define EN      1
#define IDLE    10
#define CNT     2
#define INTVL   5

static void print_error(int code);

ServerBase::ServerBase() {
    this->sfd = -1;
    this->cfd = -1;
}

void ServerBase::createServer(int port) {
    assert(port > 0);
    fprintf(stdout, "Starting Server!\n");

    struct sockaddr_in saddr;
    
    if ((this->sfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        this->sfd = -1;
        return;
    }

    setServerSockOpts();

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

    if (listen(this->sfd, MAXCONN) < 0) {
        close(this->sfd);
        this->sfd = -1;
        return;
    }

    return;
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

void ServerBase::run() {
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

        fprintf(stdout, "Connected!\n");
        connection();
        fprintf(stdout, "Disconnected!\n");
        close(this->cfd);
        this->cfd = -1;
    }
    return;
}

void ServerBase::connection() {
    fprintf(stdout, "BASE CLASS THREAD\n");

   // char *c_ip = inet_ntoa(caddr.sin_addr);
   // std::string ip(c_ip);

    std::string msg = "Connected\n";// + ip + "\n";
    sendToClient(msg);

    fd_set set;
    struct timeval timeout;
    FD_ZERO(&set);
    FD_SET(this->cfd, &set);

    while (1) {
        setClientSockOpts();
        char buf[1024] = {0};
        
        struct sigaction new_actn, old_actn;
        new_actn.sa_handler = SIG_IGN;
        sigemptyset (&new_actn.sa_mask);
        new_actn.sa_flags = 0;
        sigaction(SIGPIPE, &new_actn, &old_actn);
        int 
        if (write(this->cfd, (void *)buf, sizeof(buf)) < 0) {
            print_error(errno);
        }
        sigaction (SIGPIPE, &old_actn, NULL);
        //sendToClient(msg);
    }
    return;
}

int ServerBase::readFromClient(char *cmdline, int len) {
    assert(cmdline != NULL);
    assert(this->cfd >= 0);

    rio_t buf;
    rio_readinitb(&buf, this->cfd);

    rio_readlineb(&buf, cmdline, len);
    cmdline[strlen(cmdline)-1] = '\0';

    return 0;
}

void ServerBase::sendToClient(std::string msg) {
    assert(!msg.empty());
    assert(this->cfd >= 0);

    std::string toSend = "Server: " + msg;

    rio_writen(this->cfd, (void *)toSend.c_str(), strlen(toSend.c_str()));

    return;
}

void ServerBase::shutdown() {
    close(this->cfd);
    this->cfd = -1;
    close(this->sfd);
    this->sfd = -1;

    return;
}

int ServerBase::setClientSockOpts() {
    int flags = EN;
    if (setsockopt(this->sfd, SOL_SOCKET, SO_KEEPALIVE,
                   (const void *)&flags, sizeof(flags)) < 0) {
        print_error(errno);
        return -1;
    }
    return 0;
}

int ServerBase::setServerSockOpts() {
    int flags;

    // these if-statements setup the handshake to keep
    // alive the connection to client
    // if client does not respond, SIGPIPE is issued
    flags = EN; // enables address reuse
    if (setsockopt(this->sfd, SOL_SOCKET, SO_REUSEADDR,
                   (const void*)&flags, sizeof(flags)) < 0) {
        print_error(errno);
        return -1;
    }

    flags = IDLE; // heartbeat frequency
    if (setsockopt(this->sfd, IPPROTO_TCP, TCP_KEEPIDLE,
                   (const void*)&flags, sizeof(flags)) < 0) {
        print_error(errno);
        return -1;
    }
    flags = CNT; // defines number of missed heartbeats as dropped client
    if (setsockopt(this->sfd, IPPROTO_TCP, TCP_KEEPCNT,
                   (const void*)&flags, sizeof(flags)) < 0) {
        print_error(errno);
        return -1;
    }
    flags = INTVL; // heatbeat freq when client isn't responding
    if (setsockopt(this->sfd, IPPROTO_TCP, TCP_KEEPINTVL,
                   (const void*)&flags, sizeof(flags)) < 0) {
        print_error(errno);
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

    getsockopt(this->sfd, SOL_TCP, TCP_KEEPIDLE, &optval, (unsigned int*)&optlen);
    fprintf(stderr, "optval: %d\n", optval);
    if (optval != IDLE) {
        fprintf(stderr, "Error: TCP_KEEPIDLE not set!\n");
        fail = -1;
    }

    getsockopt(this->sfd, SOL_TCP, TCP_KEEPCNT, &optval, (unsigned int*)&optlen);
    fprintf(stderr, "optval: %d\n", optval);
    if (optval != CNT) {
        fprintf(stderr, "Error: TCP_KEEPCNT not set!\n");
        fail = -1;
    }

    getsockopt(this->sfd, SOL_TCP, TCP_KEEPINTVL, &optval, (unsigned int*)&optlen);
    fprintf(stderr, "optval: %d\n", optval);
    if (optval != INTVL) {
        fprintf(stderr, "Error: TCP_KEEPINTVL not set!\n");
        fail = -1;
    }
    
    getsockopt(this->cfd, SOL_SOCKET, SO_KEEPALIVE, &optval, (unsigned int*)&optlen);
    fprintf(stderr, "optval: %d\n", optval);
    if (optval != 1) {
        fprintf(stderr, "Error: SO_KEEPALIVE not enabled!\n");
        fail = -1;
    }

    return fail;
}

ServerBase::~ServerBase() {

}

static void print_error(int code) {
    fprintf(stderr, "\t\t\t\t%s\n", strerror(code));
    return;
}
