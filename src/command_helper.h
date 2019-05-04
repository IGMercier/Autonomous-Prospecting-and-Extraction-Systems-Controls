/*
    unabashedly taken and modified from tshlab
*/

#ifndef COMMAND_HELPER_H
#define COMMAND_HELPER_H

#define MAXLINE 1024
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
