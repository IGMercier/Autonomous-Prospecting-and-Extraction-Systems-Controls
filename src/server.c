#include "csapp.h"
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <signal.h>
#include <sys/socket.h>
#include <assert.h>
#include <pthread.h>

static void *thread(void *arg);
static int accept_data(int client_fd);

static int server_fd;

int main(int argc, char** argv) {
    char *port = NULL;

    // signal(SIGPIPE, SIG_IGN);
    
    // process args
    char c;
    while ((c = getopt(argc, argv, "-h"))) {
        switch (c) {
            case 1:
                port = optarg;
                break;
            case 'h':
                fprintf(stdout, "ARGS: -p portNumber\n");
                return -1;
            case '?':
            default:
                port = "22";
        }
    }
    assert(port != NULL);

    server_fd = open_listenfd(port);
    if (server_fd < 0) {
        fprintf(stderr, "ERROR: %s\n", strerror(errno));
        return -1;
    }

    // user connection
    pthread_t tid;

    // @TODO: for now, connects as many as it has resources for
    while (1) {
        int client_fd = -1;
        struct addrinfo server_addr = {
            .ai_addr = NULL,
            .ai_addrlen = 0
        };

        // accept() blocks until client connects
        client_fd = accept(server_fd,
                           server_addr.ai_addr,
                           &(server_addr.ai_addrlen));
        if (client_fd < 0) {
            fprintf(stderr, "ERROR: %s\n", strerror(errno));
            continue;
        }

        if (pthread_create(&tid, NULL, thread, (void *)(long)client_fd) < 0) {
            fprintf(stderr, "ERROR: %s\n", strerror(errno));
            if (client_fd < 0) {
                fprintf(stderr, "ERROR: %s\n", strerror(errno));
                continue;
            }
            continue;
        }
    }

    if (close(server_fd) < 0) {
        fprintf(stderr, "ERROR: %s\n", strerror(errno));
    }
    return 0;
}

static void *thread(void *arg) {
    if (pthread_detach(pthread_self()) < 0) {
        fprintf(stderr, "ERROR: %s\n", strerror(errno));
    }

    int client_fd = (int)(long)arg;

    while (1) {
        accept_data(client_fd);
    }

    return NULL;
}

static int accept_data(int client_fd) {
    rio_t buf;
    Rio_readinitb(&buf, client_fd);

    char msg[RIO_BUFSIZE];

    Rio_readlineb(&buf, msg, RIO_BUFSIZE);
    fprintf(stdout, "Received: %s", msg);

    return 0;
}


