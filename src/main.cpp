#include "APES.h"
#include <stdio.h>
#include <errno.h>
#include <signal.h>

APES robot;
static void sigint_handler(int sig);

int main() {
    signal(SIGINT, sigint_handler);


    if (robot.setup() == -1) {
        robot.finish();
    }
    while(1)
    {
    	int data = robot.measMCP3008(7);
    	printf("adc val = %d\n", data);
    	//robot.measWOB();
    }
    
    return 0;
}

static void sigint_handler(int sig) {
    robot.finish();
    exit(0);
}
