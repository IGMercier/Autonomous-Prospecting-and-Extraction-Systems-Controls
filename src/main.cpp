#include "APES.h"
#include <stdio.h>
#include <errno.h>
#include <signal.h>

APES robot;
static void sigint_handler(int sig);

int main() {
    signal(SIGINT, sigint_handler);


    robot.setup();
    int data = robot.measMCP3008(0);
    printf("adc val = %d\n", data);
    robot.measWOB();
    
    //robot.finish();
   	
    return 0;
}

static void sigint_handler(int sig) {
    robot.finish();
    exit(0);
}
