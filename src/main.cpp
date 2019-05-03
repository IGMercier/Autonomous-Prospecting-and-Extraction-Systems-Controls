#include "APES.h"
#include <stdio.h>
#include <errno.h>
#include <signal.h>

APES robot;
static void sigint_handler(int sig);

int checkConnection() {

}

int main() {
    signal(SIGINT, sigint_handler);

    if (robot.setup() == -1) {
        fprintf(stderr, "ERROR: APES system setup failure!");
        fprintf(stderr, " Shutting down!\n");
        robot.finish();
    }






    while(checkConnection())
    {
    	int data = robot.measMCP3008(0, 7);
    	printf("adc val = %d\n", data);
    	//robot.measWOB();
    }

    fprintf(stderr, "ERROR: APES system disconnected!");
    fprintf(stderr, " Shutting down!\n");
    robot.finish()
    return -1;
}

static void sigint_handler(int sig) {
    robot.finish();
    exit(0);
}
