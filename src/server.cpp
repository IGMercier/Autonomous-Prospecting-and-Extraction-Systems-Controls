#include "commands.h"
//#include "components.h"
//#include "APES.h"
//#include <unistd.h>
//#include <stdlib.h>
//#include <stdio.h>
//#include <string.h>
//#include <string>
//#include <cstring>
//#include <errno.h>
//#include <signal.h>
//#include <sys/socket.h>
//#include <assert.h>
//#include <pthread.h>

/*
    Setup Server
    Wait for input
    Evaluate input and add to job list, wait for more input
    separately execute each job in job list
*/

using std::string;

static int server_fd = -1;
static int client_fd = -1;
static volatile int disconnected = 1;
//APES robot;

static int serverSetup(char *port);
static int clientSetup();
static void readFromClient();
static int sendToClient(const char *msg);
static int eval(const char *cmdline);
static int command(token *tk);
static void shutdown();

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
        readFromClient();
	    if (disconnected) {
            // robot.standby();
            
            // keep trying to connect to client.
            // in this state, the loop should really
            // only run once
            clientSetup();
	    }
    }


    // control should never reach here
    shutdown();
    return -1;
}

/*
    SERVER/CLIENT FUNCTIONS
*/
void serverSetup(char *port) {
    // server setup
    struct addrinfo hints, *listp, *p;
    int optval;
    
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    //hints.ai_flags = AI_PASSIVE;
    hints.ai_protocol = 0;
    hints.ai_canonname = NULL;
    hints.air_addr = NULL;
    hints.air_next = NULL;

    int rc;
    if ((rc = getaddrinfo(NULL, port, &hints, &listp)) != 0) {
        // not reached
        return;
    }

    for (p = listp; p != NULL; p = p->ai_next) {
        server_fd = socket(p->ai_family,
                           p->ai_socktype,
                           p->ai_protocol);
        if (server_fd < 0) { continue; }

        //setsockopt(server_fd, SOL_SOCKET,
        //           SO_REUSEADDR, (const void *) &optval,
        //           sizeof(int));

        if (bind(server_fd, p->ai_addr, p->ai_addrlen) == 0) {
            break; // found a socket
        }

        close(server_fd);
    }
    freeaddrinfo(listp);
    if (!p) {
        return;
    }

    if (listen(server_fd, LISTENQ) < 0) {
        close(server_fd);
        return;
    }
    return;
}

int clientSetup() {
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

    return 0;
}

/*
    EVALUATION FUNCTIONS
*/
void readFromClient() {
    size_t n;
    char cmdline[MAXLINE];
    
    // read/eval loop
    while ((n = read(client_fd, cmdline, MAXLINE)) != 0) {
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

int sendToClient(const char *msg) {
    write(client_fd, msg, strlen(msg));
    return 0;
}

void shutdown() {
    //robot.finish();

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

    string msg;

    switch (command) {
        case START:
            // setup robot and retry on fail
            //while (robot.setup() < 0) {
            //    fprintf(stderr, "ERROR: APES system setup failure!\n");
            //    fprintf(stdout, "Retrying...\n");
            //    robot.finish();
            //}

            // put in standby
            //robot.standby();
            msg = "ROBOT STARTED\n";
            sendToClient(msg.c_str());
            return 1;
        case HELP:
            sendToClient(listCommands());
            return 1;
        case QUIT:
            // shuts down everything
            shutdown();
            return 1;
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
            /*
            float temp;
            temp = robot.read_temp();
            printf(stdout, "Temp (@time): %f\n", temp);
            */
            msg = "TEMP MODE\n";
            sendToClient(msg.c_str());
            return 1;
        case DTEMP:
            /*
            float dtemp;
            dtemp = robot.D_temp();
            printf(stdout, "Temp since init: %f\n", dtemp);
            */
            msg = "DTEMP MODE\n";
            sendToClient(msg.c_str());
            return 1;
        case CURR:
            /*
            float curr;
            curr = robot.read_curr();
            printf(stdout, "Curr (@time): %f\n", curr);
            */
            msg = "CURR MODE\n";
            sendToClient(msg.c_str());
            return 1;
        case LEVEL:
            /*
            int level;
            level = robot.read_level();
            printf(stdout, "Level (@time): %d\n", level);
            */
            msg = "LEVEL MODE\n";
            sendToClient(msg.c_str());
            return 1;
        case STANDBY:
            //robot.standby();
            return 1;
        case WOB:
            /*
            float force;
            force = robot.read_wob();
            printf(stdout, "Force (@time): %f\n", force);
            */
            return 1;
        case DATA:
            //robot.read_data();
        case MOTOR_DRIVE:
            //robot.motor_drive();
            return 1;
        case MOTOR_STOP:
            //robot.motor_stop();
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

