/*
    unabashedly taken and modified from tshlab
*/

#ifndef __COMMAND_HELPER_H__
#define __COMMAND_HELPER_H__

#define MAXARGS  128

typedef enum parseline_return {
    PARSELINE_FG,
    PARSELINE_BG,
    PARSELINE_EMPTY,
    PARSELINE_ERROR
} parseline_return;

typedef enum command_state {
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

parseline_return parseline(const char *cmdline, token *tk);

#endif
