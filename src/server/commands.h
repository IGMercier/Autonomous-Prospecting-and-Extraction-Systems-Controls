
#ifndef _COMMANDS_H_
#define _COMMANDS_H_

/*
    COMMAND & PARSING DEFINITIONS AND HELPER FUNCTIONS
*/
#define MAXARGS  128
#define MAXLINE  8192

typedef enum command_state {
    START,
    HELP,
    DATA,
    NONE,
    STANDBY,
    TEMP,
    DTEMP,
    CURR,
    WLEVEL,
    WOB,
    MOTOR_DRIVE,
    MOTOR_STOP,
    DRILL_RUN,
    DRILL_STOP,
    DRILL_CYCLE,
    AUTO_ON,
    AUTO_OFF,
    QUIT
} command_state;

typedef struct token {
    char text[MAXLINE]; 
    int argc;
    char *argv[MAXARGS];
    command_state command;
} token;

int parseline(const char *cmdline, token *tk);

#endif
