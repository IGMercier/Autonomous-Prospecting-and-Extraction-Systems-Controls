#include "commands.h"
//#include "components.h"
#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <cstring>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <errno.h>
#include <assert.h>
//#include "APES.h"
//#include <string.h>
//#include <cstring>
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

static int serverSetup(int port);
static int clientSetup();
static int readFromClient();
static int sendToClient(const char *msg);
static int eval(const char *cmdline, token *tk);
static int command(token *tk);
static void shutdown();

typedef struct sockaddr SA;

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
    int port;
    if (argc < 2) {
        port = atoi("16778");
    } else {
        port = atoi(argv[1]);
    }

    fprintf(stdout, "Attempting serverSetup! ");
    while ((server_fd = serverSetup(port)) < 0) {
        fprintf(stderr, "ERROR: %s\n", strerror(errno));
        fprintf(stdout, "Retrying...\n");
        
        // retry to setup
    }
    fprintf(stdout, "Finished serverSetup!\n");
    fprintf(stdout, "Attempting clientSetup! ");

    while (clientSetup() < 0) { fprintf(stderr, "Retrying client setup!\n"); }
    fprintf(stdout, "Finished clientSetup!\n");
    
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
static int serverSetup(int port) {
    int optval = 1;
    struct sockaddr_in serveraddr;
    int sfd;

    if ((sfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        return -1;
    }

    if (setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR,
                   (const void*)&optval, sizeof(int)) < 0) {
        return -1;
    }

    bzero((char *)&serveraddr, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
    serveraddr.sin_port = htons((unsigned short)port);

    if (bind(sfd, (SA*)&serveraddr, sizeof(serveraddr)) < 0) {
        return -1;
    }

    if (listen(sfd, 1024) < 0) {
        return -1;
    }

    return sfd;

}

static int clientSetup() {
    struct sockaddr_in clientaddr;
    int clientlen;

    clientlen = sizeof(clientaddr);

    // accept() blocks until client connects
    client_fd = accept(server_fd,
                       (SA*)&clientaddr.sin_addr.s_addr,
                       sizeof(clientaddr.sin_addr.s_addr),
                       AF_INET);
    if (client_fd < 0) {
        fprintf(stderr, "ERROR: %s\n", strerror(errno));
        return -1;
    }

	disconnected = 0;
    //string msg = "Connected...!\n";
    //sendToClient(msg.c_str());
    //fprintf(stdout, msg.c_str());

    return 0;
}

/*
    EVALUATION FUNCTIONS
*/
static int readFromClient() {
    size_t n;
    char cmdline[MAXLINE];
    
    // read/eval loop
    while ((n = read(client_fd, cmdline, MAXLINE)) != 0) {
        cmdline[strlen(cmdline)-1] = '\0';
        fprintf(stdout, "Received: %s\n", cmdline);

        token tk;
        int eval_result = eval(cmdline, &tk);

        if (eval_result == 0) {
            fprintf(stdout, "Unknown Command\n\n");
            //string msg = "ERROR: Unknown command!\n";
            //sendToClient(msg.c_str());
        }

        memset(cmdline, 0, MAXLINE);
	
    }

    if (close(client_fd) < 0) {
        fprintf(stderr, "ERROR: %s\n", strerror(errno));
    }
    disconnected = 1;
    fprintf(stdout, "Disconnected...\n");
    return 0;
}

static int sendToClient(const char *msg) {
    write(client_fd, msg, strlen(msg));
    return 0;
}

static void shutdown() {
    //robot.finish();

    //string msg = "Shutting down!\n";
    //sendToClient(msg.c_str());
    //fprintf(stdout, msg.c_str());

    fprintf(stdout, "SHUTTING DOWN\n");

    if (close(client_fd) < 0) {
        fprintf(stderr, "ERROR: %s\n", strerror(errno));
    }
    if (close(server_fd) < 0) {
        fprintf(stderr, "ERROR: %s\n", strerror(errno));
    }
    exit(0);
}

static int eval(const char *cmdline, token *tk) {
    assert(cmdline != NULL);
    assert(tk != NULL);

    parseline(cmdline, tk);

    return command(tk);
}

static int command(token *tk) {

    command_state command = tk->command;
    fprintf(stdout, "%s\n", tk->text);

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
            //msg = "ROBOT STARTED\n";
            //sendToClient(msg.c_str());
            return 1;
        case HELP:
            //sendToClient(listCommands());
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
            //msg = "TEMP MODE\n";
            //sendToClient(msg.c_str());
            return 1;
        case DTEMP:
            /*
            float dtemp;
            dtemp = robot.D_temp();
            printf(stdout, "Temp since init: %f\n", dtemp);
            */
            //msg = "DTEMP MODE\n";
            //sendToClient(msg.c_str());
            return 1;
        case CURR:
            /*
            float curr;
            curr = robot.read_curr();
            printf(stdout, "Curr (@time): %f\n", curr);
            */
            //msg = "CURR MODE\n";
            //sendToClient(msg.c_str());
            return 1;
        case LEVEL:
            /*
            int level;
            level = robot.read_level();
            printf(stdout, "Level (@time): %d\n", level);
            */
            //msg = "LEVEL MODE\n";
            //sendToClient(msg.c_str());
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
        case NOP:
            return 1;
        case NONE:
        default:
            // not a built-in command
            return 0;
    }
}

