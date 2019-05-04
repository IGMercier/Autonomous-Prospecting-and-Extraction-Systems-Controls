#include "csapp.h"
//#include "command_helper.h"
//#include "APES.h"
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <sys/socket.h>
#include <assert.h>
#include <pthread.h>

static volatile int disconnected = 1;
//APES robot;

static int serverSetup(char *port);
static int clientSetup(int server_fd);
static void *thread(void *arg);
void eval(const char *cmdline);

int main(int argc, char** argv) {
    /* @TODO:
       [X] setup robot
       [X] put robot in standby
       [] when connected:
         [] accept commands
	     [] execute commands
       [X] when disconnected:
         [X] put robot in standby
     */

    // setup robot
    /*
    if (robot.setup() < 0) {
        fprintf(stderr, "ERROR: APES system setup failure!\n");
        fprintf(stderr, " Shutting down!\n");
        robot.finish();
    }

    // put in standby
    robot.standby();
    */

    int server_fd;
    char *port = argv[1];
    server_fd = serverSetup(port);
    if (server_fd < 0) {
        fprintf(stderr, "ERROR: %s\n", strerror(errno));
        return -1;
    }

    clientSetup(server_fd);
    
    while (1) {
	    if (disconnected) {
    		//fprintf(stderr, "Disconnected...\n");
            //robot.standby();
            
            // keep trying to connect to client
            clientSetup(server_fd);
	    }
    }


    // control should never reach here
    if (close(server_fd) < 0) {
        fprintf(stderr, "ERROR: %s\n", strerror(errno));
    }
    return -1;
}

static void *thread(void *arg) {
    if (pthread_detach(pthread_self()) < 0) {
        fprintf(stderr, "ERROR: %s\n", strerror(errno));
    }

    int client_fd = (int)(long)arg;
    size_t n;
    rio_t buf;
    Rio_readinitb(&buf, client_fd);
    char cmdline[RIO_BUFSIZE];
    
    // read/eval loop
    while((n = Rio_readlineb(&buf, cmdline, RIO_BUFSIZE)) != 0) {
        fprintf(stdout, "Received: %s", cmdline);
        cmdline[strlen(cmdline)-1] = '\0';
        //eval(cmdline);
        fflush(stdout);
        //token *tk = eval(command);
	    //parse_token(tk);
	
    }

    // acceptData(client_fd);
    if (close(client_fd) < 0) {
        fprintf(stderr, "ERROR: %s\n", strerror(errno));
    }
    disconnected = 1;
    return NULL;
}

// server functions
static int serverSetup(char *port) {
    // server setup
    int server_fd = open_listenfd(port);
    return server_fd;
}

static int clientSetup(int server_fd) {
    // client connection
    int client_fd = -1;
    pthread_t tid;
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

    // creates new thread that handles client input
    if (pthread_create(&tid, NULL, thread, (void *)(long)client_fd) < 0) {
        fprintf(stderr, "ERROR: %s\n", strerror(errno));
        if (client_fd < 0) {
            fprintf(stderr, "ERROR: %s\n", strerror(errno));
        }
    }

    return 0;
}

void eval(const char *cmdline) {
    parseline_return parse_result;
    struct cmdline_tokens token;

    parse_result = parseline(cmdline, &token);
}

