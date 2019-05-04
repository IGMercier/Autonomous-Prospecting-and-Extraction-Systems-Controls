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
static volatile int disconnected = 1;

int main(int argc, char** argv) {
    char *port = argv[1];

    server_fd = open_listenfd(port);
    if (server_fd < 0) {
        fprintf(stderr, "ERROR: %s\n", strerror(errno));
        return -1;
    }

    // user connection
    pthread_t tid;

    // @TODO: for now, connects as many as it has resources for
    int client_fd = -1;
    while (client_fd < 0) {
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

	disconnected = 0;
        fprintf(stderr, "Connected...\n");
    }


    if (pthread_create(&tid, NULL, thread, (void *)(long)client_fd) < 0) {
        fprintf(stderr, "ERROR: %s\n", strerror(errno));
        if (client_fd < 0) {
            fprintf(stderr, "ERROR: %s\n", strerror(errno));
        }
    }
    while (disconnected == 0) { }
    fprintf(stderr, "Disconnected...\n");
    // @TODO: Put system into standby


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

    accept_data(client_fd);
    if (close(client_fd) < 0) {
        fprintf(stderr, "ERROR: %s\n", strerror(errno));
    }
    disconnected = 1;
    return NULL;
}

static int accept_data(int client_fd) {
    size_t n;
    rio_t buf;
    Rio_readinitb(&buf, client_fd);

    char command[RIO_BUFSIZE];

    while((n = Rio_readlineb(&buf, command, RIO_BUFSIZE)) != 0) {
        fprintf(stdout, "Received: %s", command);
	// parse command
	
    }

    return 0;
}


