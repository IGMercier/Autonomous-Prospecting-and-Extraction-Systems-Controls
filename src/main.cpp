#include "APES.h"
#include <stdio.h>
#include <errno.h>
#include <signal.h>

APES robot;
static void sigint_handler(int sig);

int main() {
    int isConnected;

    signal(SIGINT, sigint_handler);

    if (robot.setup() == -1) {
        robot.finish();
    }
    while(isConnected)
    {
    	int data = robot.measMCP3008(0, 7);
    	printf("adc val = %d\n", data);
    	//robot.measWOB();
    }

    fprintf(stderr, "ERROR: APES system disconnected! Shutting down!\n");
    robot.finish()
    return 0;
}

static void sigint_handler(int sig) {
    robot.finish();
    exit(0);
}
