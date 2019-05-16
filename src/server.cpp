#include "csapp.h"
#include "commands.h"
#include "components.h"
#include "APES.h"
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <sys/socket.h>
#include <assert.h>
#include <pthread.h>

static int server_fd;
static int client_fd = -1;
static volatile int disconnected = 1;
APES robot;

int serverSetup(char *port);
int clientSetup();
void connected();
int sendToClient(const char *msg);
int eval(const char *cmdline);
int command(token *tk);
void shutdown();

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


    char *port;
    if (argc < 2) {
        port = "16778";
    } else {
        port = argv[1];
    }

    server_fd = serverSetup(port);
    while (server_fd < 0) {
        fprintf(stderr, "ERROR: %s\n", strerror(errno));
        fprintf(stdout, "Retrying...\n");
        
        // retry to setup
        server_fd = serverSetup(port);
    }

    clientSetup();
    
    while (1) {
        connected();
	    if (disconnected) {
            robot.standby();
            
            // keep trying to connect to client.
            // in this state, the loop should really
            // only run once
            clientSetup(server_fd);
	    }
    }


    // control should never reach here
    shutdown();
    return -1;
}

void connected() {
    //if (pthread_detach(pthread_self()) < 0) {
    //    fprintf(stderr, "ERROR: %s\n", strerror(errno));
    //}

    //struct args param = (struct args)(long)arg;
    //int server_fd = param.server_fd;
    //int client_fd = param.client_fd;
    
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
            string msg = "ERROR: Unknown command!\n";
            sendToClient(msg.c_str());
        }
	
    }

    if (close(client_fd) < 0) {
        fprintf(stderr, "ERROR: %s\n", strerror(errno));
    }
    disconnected = 1;
    fprintf(stdout, "Disconnected...\n");
    return;
}

// server functions
static int serverSetup(char *port) {
    // server setup
    int server_fd = open_listenfd(port);
    return server_fd;
}

static int clientSetup() {
    // client connection
    //int client_fd = -1;
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
    string msg = "Connected...!\n";
    sendToClient(msg.c_str());
    fprintf(stdout, msg.c_str());

    //struct args param = {
    //    .server_fd = server_fd,
    //    .client_fd = client_fd
    //};

    /*
    // creates new thread that handles client input
    if (pthread_create(&tid, NULL, thread, (void *)(long)param) < 0) {
        fprintf(stderr, "ERROR: %s\n", strerror(errno));
        if (close(client_fd) < 0) {
            fprintf(stderr, "ERROR: %s\n", strerror(errno));
        }
        disconnected = 1;
        return -1;
    }*/

    return 0;
}

int sendToClient(const char *msg) {
    return rio_writen(client_fd, msg, strlen(msg));
}

void shutdown() {
    robot.finish();

    string msg = "Shutting down!\n";
    sendToClient(msg.c_str());
    fprintf(stdout, msg.c_str());

    if (close(client_fd) < 0) {
        fprintf(stderr, "ERROR: %s\n", strerror(errno));
    }
    if (close(server_fd) < 0) {
        fprintf(stderr, "ERROR: %s\n", strerror(errno));
    }
    exit(0);
}

int eval(const char *cmdline) {
    token tk;

    if (parseline(cmdline, &tk) < 0) {
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
        case START:
            // setup robot and retry on fail
            while (robot.setup() < 0) {
                fprintf(stderr, "ERROR: APES system setup failure!\n");
                fprintf(stdout, "Retrying...\n");
                robot.finish();
            }

            // put in standby
            robot.standby();
            break;
        case HELP:
            sendToClient(listCommands());
            break;
        case QUIT:
            // shuts down everything
            shutdown();
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
            float temp;
            temp = robot.read_temp();
            printf(stdout, "Temp (@time): %f\n", temp);
            return 1;
        case DTEMP:
            float dtemp;
            dtemp = robot.D_temp();
            printf(stdout, "Temp since init: %f\n", dtemp);
            return 1;
        case CURR:
            float curr;
            curr = robot.read_curr();
            printf(stdout, "Curr (@time): %f\n", curr);
            return 1;
        case LEVEL:
            int level;
            level = robot.read_level();
            printf(stdout, "Level (@time): %d\n", level);
            return 1;
        case STANDBY:
            robot.standby();
            return 1;
        case WOB:
            float force;
            force = robot.read_wob();
            printf(stdout, "Force (@time): %f\n", force);
            return 1;
        case DATA:
            robot.read_data();
        case MOTOR_DRIVE:
            robot.motor_drive();
            return 1;
        case MOTOR_STOP:
            robot.motor_stop();
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
            // not a built-in command
            return 0;
    }
}

