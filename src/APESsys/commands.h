
#ifndef _COMMANDS_H_
#define _COMMANDS_H_

/*
    COMMAND & PARSING DEFINITIONS AND HELPER FUNCTIONS
*/
#define MAXARGS  128
#define MAXLINE  8192

typedef enum command_state {
    START,
    STANDBY,
    DATA,
    HELP,
    QUIT,
    AUTO_ON,
    AUTO_OFF,
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
    NONE
} command_state;

typedef enum builtin {
    BUILTIN_FG,
    BUILTIN_BG,
    BUILTIN_JOBS,
    BUILTIN_NONE
} builtin;

typedef struct command_token {
    command_state command;
} command_token;

typedef struct parse_token {
    int argc;
    char *argv[MAXARGS];
    builtin bcomm;
} parse_token;

//int parseline(const char *cmdline, token *tk);

#endif
