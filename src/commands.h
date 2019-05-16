/*
    unabashedly taken and modified from tshlab
*/

#ifndef __COMMAND_HELPER_H__
#define __COMMAND_HELPER_H__

#define MAXARGS  128
#define MAXLINE  8192

#include "csapp.h"

typedef enum command_state {
    START,
    HELP,
    DATA,
    NONE,
    STANDBY,
    TEMP,
    DTEMP,
    CURR,
    LEVEL,
    WOB,
    MOTOR_DRIVE,
    MOTOR_STOP,
    DRILL_RUN,
    DRILL_STOP,
    DRILL_CYCLE,
    AUTO,
    QUIT
} command_state;

typedef struct token {
    char text[MAXLINE]; 
    int argc;
    char *argv[MAXARGS];
    command_state command;
} token;

int parseline(const char *cmdline, token *tk);
const char* listCommands();

#endif
