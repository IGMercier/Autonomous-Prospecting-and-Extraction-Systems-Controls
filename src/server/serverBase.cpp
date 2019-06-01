#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h> // for AF_INET
#include <netdb.h> // for ip_ntoa
#include <arpa/inet.h> // for ip_ntoa
#include <netinet/tcp.h>
#include <assert.h>
#include <errno.h>

#include <fcntl.h>

#include "serverBase.h"
#include "../misc/flags.h"

#define EN      1
#define IDLE    10
#define CNT     2
#define INTVL   5

ServerBase::ServerBase() {
    this->sfd = -1;
    this->cfd = -1;
}

void ServerBase::createServer(int port) {
    assert(port > 0);
    print("Starting Server!");

    struct sockaddr_in saddr;
    
    if ((this->sfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        this->sfd = -1;
        return;
    }
   
    setClientSockOpts();
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

    // accept() blocks until client connects
    assert(this->sfd >= 0);

    int rc = accept(this->sfd,
                    (struct sockaddr *)&caddr,
                    &caddr_size);
    if (rc < 0) { return -1; }
    this->cfd = rc;
    return 0;
}

void ServerBase::execute() {
   // char *c_ip = inet_ntoa(caddr.sin_addr);
   // std::string ip(c_ip);

    std::string msg = "Server: Connected\n";
    sendToClient(msg);
    while (1) {
        setServerSockOpts();
        setClientSockOpts();
        msg = "in loop\n";
        sendToClient(msg);
    }

    close(this->cfd);
    this->cfd = -1;
    return;
}

int ServerBase::setServerSockOpts() {
    int flags;

    // these if-statements setup the handshake to keep
    // alive the connection to client
    // if client does not respond, SIGPIPE is issued
    flags = EN; // enables address reuse
    if (setsockopt(this->sfd, SOL_SOCKET, SO_REUSEADDR,
                   (const void*)&flags, sizeof(flags)) < 0) {
        //print(strerror(errno));
        return -1;
    }

    flags = 1;
    if (setsockopt(this->sfd, SOL_SOCKET, SO_KEEPALIVE,
                   (const void*)&flags, sizeof(flags)) < 0) {
        //print(strerror(errno));
        return -1;
    }
    

    flags = IDLE; // heartbeat frequency
    if (setsockopt(this->sfd, IPPROTO_TCP, TCP_KEEPIDLE,
                   (const void*)&flags, sizeof(flags)) < 0) {
        //print(strerror(errno));
        return -1;
    }
    flags = CNT; // defines number of missed heartbeats as dropped client
    if (setsockopt(this->sfd, IPPROTO_TCP, TCP_KEEPCNT,
                   (const void*)&flags, sizeof(flags)) < 0) {
        //print(strerror(errno));
        return -1;
    }
    flags = INTVL; // heatbeat freq when client isn't responding
    if (setsockopt(this->sfd, IPPROTO_TCP, TCP_KEEPINTVL,
                   (const void*)&flags, sizeof(flags)) < 0) {
        //print(strerror(errno));
        return -1;
    }

    return 0;

}

int ServerBase::setClientSockOpts() {
    int flags = 1;
    if (setsockopt(this->cfd, SOL_SOCKET, SO_KEEPALIVE,
                   (const void *)&flags, sizeof(flags)) < 0) {
        //print(strerror(errno));
        return -1;
    }
    return 0;

}

int ServerBase::checkSockOpts() {
    int optval, optlen;

    int fail = 0;

    getsockopt(this->sfd, SOL_SOCKET, SO_REUSEADDR, &optval,(unsigned int*)&optlen);
    if (optval != 1) {
        //print(strerror(errno));
        fail = -1;
    }

    getsockopt(this->sfd, IPPROTO_TCP, TCP_KEEPIDLE, &optval, (unsigned int*)&optlen);
    if (optval != IDLE) {
        //print(strerror(errno));
        fail = -1;
    }

    getsockopt(this->sfd, IPPROTO_TCP, TCP_KEEPCNT, &optval, (unsigned int*)&optlen);
    if (optval != CNT) {
        //print(strerror(errno));
        fail = -1;
    }

    getsockopt(this->sfd, IPPROTO_TCP, TCP_KEEPINTVL, &optval, (unsigned int*)&optlen);
    if (optval != INTVL) {
        //print(strerror(errno));
        fail = -1;
    }

    return fail;
}

int ServerBase::readFromClient(char *cmdline) {
    assert(cmdline != NULL);
    assert(this->cfd >= 0);
    
    int rc;
    if ((rc = read(this->cfd, cmdline, MAXLINE)) < 0) {
        if (errno == EAGAIN) {
            print(strerror(errno));
            return -1;
        } else if (errno == EWOULDBLOCK) {
            print(strerror(errno));
            return -1;
        } else if (errno == EBADF) {
            print(strerror(errno));
            return -1;
        } else if (errno == EFAULT) {
            print(strerror(errno));
            return -1;
        } else if (errno == EINTR) {
            print(strerror(errno));
            return -1;
        } else if (errno == EIO) {
            print(strerror(errno));
            return -1;
        } else if (errno == ENOTCONN) {
            print(strerror(errno));
            return -1;
        } else if (errno == ECONNRESET) {
            print(strerror(errno));
            return -1;
        } else if (errno == EPIPE) {
            print(strerror(errno));
            return -1;
        } else if (errno == ETIMEDOUT) {
            print(strerror(errno));
            return -1;
        } else { 
            print(strerror(errno));
            return -1;
        }
    } else if (rc > 0) {
        return 1;
    }
    return 0;
}

int ServerBase::sendToClient(std::string msg) {
    assert(!msg.empty());
    assert(this->cfd >= 0);

    int len = msg.length();
    const char *tmp = msg.c_str();

    while (len > 0) {
        int rc = write(this->cfd, tmp, len);
	if (rc < 0) {
            if (errno == EAGAIN) {
                print(strerror(errno));
                return -1;
            } else if (errno == EWOULDBLOCK) {
                print(strerror(errno));
                return -1;
            } else if (errno == EBADF) {
                print(strerror(errno));
                return -1;
            } else if (errno == EFAULT) {
                print(strerror(errno));
                return -1;
            } else if (errno == EINTR) {
                print(strerror(errno));
                return -1;
            } else if (errno == EIO) {
                print(strerror(errno));
                return -1;
            } else if (errno == ENOTCONN) {
                print(strerror(errno));
                return -1;
            } else if (errno == ECONNRESET) {
                print(strerror(errno));
                return -1;
            } else if (errno == EPIPE) {
                print(strerror(errno));
                return -1;
            } else if (errno == ETIMEDOUT) {
                print(strerror(errno));
                return -1;
            } else { 
                print(strerror(errno));
                return -1;
            }
	}
        tmp += rc;
	len -= rc;
    }
    return 0;
}

void ServerBase::print(std::string msg) {
    const char *tmp = msg.c_str();
    if (VERBOSE) {
        printf("\t\t\t\t%s\n", tmp);
    }
}

void ServerBase::shutdown() {
    close(this->cfd);
    close(this->sfd);
    exit(0);
}

ServerBase::~ServerBase() {}
