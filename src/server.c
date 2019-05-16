#include "csapp.h"
#include "command_helper.h"
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

int serverSetup(char *port);
int clientSetup(int server_fd);
static void *thread(void *arg);
int eval(const char *cmdline);
int command(token *tk);
void shutdown(int server_fd, int client_fd);

struct args {
    int server_fd;
    int client_fd;
};

int main(int argc, char** argv) {
    /* @TODO:
       [X] setup robot
       [X] put robot in standby
       [] when connected:
         [X] accept commands
	     [] execute commands
       [X] when disconnected:
         [X] put robot in standby
     */

    // setup robot
    /*
    if (robot.setup() < 0) {
        fprintf(stderr, "ERROR: APES system setup failure!\n");
        fprintf(stdout, "Shutting down!\n");
        robot.finish();
        return -1; // @TODO: retry
    }

    // put in standby
    robot.standby();
    */

    char *port;
    if (argc < 2) {
        port = "16778";
    } else {
        port = argv[1];
    }

    int server_fd;
    server_fd = serverSetup(port);
    if (server_fd < 0) {
        fprintf(stderr, "ERROR: %s\n", strerror(errno));
        fprintf(stdout, "Shutting down!\n");
        // robot.finish();
        return -1; // @TODO: retry
    }

    clientSetup(server_fd);
    
    while (1) {
	    if (disconnected) {
            //robot.standby();
            
            // keep trying to connect to client.
            // in this state, the loop should really
            // only run once
            clientSetup(server_fd);
	    }
    }


    // control should never reach here
    if (close(server_fd) < 0) {
        fprintf(stderr, "ERROR: %s\n", strerror(errno));
        fprintf(stdout, "Shutting down!\n");
    }
    return -1;
}

static void *thread(void *arg) {
    if (pthread_detach(pthread_self()) < 0) {
        fprintf(stderr, "ERROR: %s\n", strerror(errno));
    }

    struct args param = (struct args)(long)arg;
    int server_fd = param.server_fd;
    int client_fd = param.client_fd;
    
    size_t n;
    rio_t buf;
    Rio_readinitb(&buf, client_fd);
    char cmdline[RIO_BUFSIZE];
    
    // read/eval loop
    while((n = Rio_readlineb(&buf, cmdline, RIO_BUFSIZE)) != 0) {
        fprintf(stdout, "Received: %s", cmdline);
        cmdline[strlen(cmdline)-1] = '\0';

        int eval_result = eval(cmdline);
        if (eval_result == 0) {
            const char *msg = "ERROR: Unknown command!\n";
            rio_writen(client_fd, msg, strlen(msg));
        } else if (eval_result == -1) {
            shutdown(server_fd, client_fd);
            exit(0);
        }
	
    }

    if (close(client_fd) < 0) {
        fprintf(stderr, "ERROR: %s\n", strerror(errno));
    }
    disconnected = 1;
    fprintf(stdout, "Disconnected...\n");
    return NULL;
}

// server functions
static int serverSetup(char *port) {
    // server setup
    int server_fd = open_listenfd(port);
    return server_fd;
}

static int clientSetup(int server_fd) {
    /* @TODO: refactor this to allow only ONE
        connection
    */
    // client connection
    int client_fd = -1;
    pthread_t tid;

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
        return -1;
    }

	disconnected = 0;
    const char *msg = "Connected...!\n";
    rio_writen(client_fd, msg, strlen(msg));
    fprintf(stdout, msg);

    struct args param = {
        .server_fd = server_fd,
        .client_fd = client_fd
    };

    // creates new thread that handles client input
    if (pthread_create(&tid, NULL, thread, (void *)(long)param) < 0) {
        fprintf(stderr, "ERROR: %s\n", strerror(errno));
        if (close(client_fd) < 0) {
            fprintf(stderr, "ERROR: %s\n", strerror(errno));
        }
        disconnected = 1;
        return -1;
    }

    return 0;
}

void shutdown(int server_fd, int client_fd) {
    // robot.finish();
    const char *msg = "Shutting down!\n";
    rio_writen(client_fd, msg, strlen(msg));

    fprintf(stdout, msg);
    if (close(client_fd) < 0) {
        fprintf(stderr, "ERROR: %s\n", strerror(errno));
    }
    if (close(server_fd) < 0) {
        fprintf(stderr, "ERROR: %s\n", strerror(errno));
    }
}

int eval(const char *cmdline) {
    parseline_return parse_result;
    token tk;

    parse_result = parseline(cmdline, &tk);
    if (parse_result == PARSELINE_ERROR || parse_result == PARSELINE_EMPTY) {
        return 0;
    }

    if (!command(&tk)) {
        return -1;
    }
    return 0;
}

int command(token *tk) {
    command_state command = tk->command;

    switch (command) {
        case QUIT:
            // shuts down everything
            return -1;
        case AUTO:
            // runs things automatically
            /*
                @TODO:
                Will this need to create a new thread?
                We want this to run in the background,
                but we also want to be able to input more
                commands...
                If we spawn a process, APES system will be copied,
                so we dont want that...
            */
            return 1;
        case TEMP:
            // robot.read_temp(robot->thermo);
            return 1;
        case DTEMP:
            // robot.D_temp(robot->thermo);
            return 1;
        case CURR:
            // robot.read_curr(robot->ammeter);
            return 1;
        case LEVEL:
            // robot.read_level(robot->wlevel);
            return 1;
        case STANDBY:
            // robot.standby();
            return 1;
        case WOB:
            // robot.read_wob(robot->wob);
            return 1;
        case MOTOR_DRIVE:
            // drives motor
            return 1;
        case MOTOR_STOP:
            // stops motor
            return 1;
        // do things for switch
        case DRILL_RUN:
            // runs drill
            return 1;
        case DRILL_STOP:
            // stops drill
            return 1;
        case DRILL_CYCLE:
            // runs drill at duty cycle
            return 1;
        case NONE:
        default:
            // not a builtin command
            return 0;
    }
}

