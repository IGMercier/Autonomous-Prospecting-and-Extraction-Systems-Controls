
#ifndef _COMMANDS_H_
#define _COMMANDS_H_

/*
    COMMAND & PARSING DEFINITIONS AND HELPER FUNCTIONS
*/
#define MAXARGS  128

typedef enum command_state {
    START,
    STANDBY,
    DATA,
    HELP,
    QUIT,
    AUTO_ON,
    AUTO_OFF,
    SOL_0_OPEN,
    SOL_0_CLOSE,
    SOL_1_OPEN,
    SOL_1_CLOSE,
    HEATER_0_ON,
    HEATER_0_OFF,
    HEATER_1_ON,
    HEATER_1_OFF,
    RELAY_0_ON,
    RELAY_0_OFF,
    RELAY_1_ON,
    RELAY_1_OFF,
    TEMP,
    DTEMP,
    CURR,
    WLEVEL,
    WOB,
    MOTOR_Z_DRIVE,
    MOTOR_Z_STOP,
    PUMP_DRIVE,
    PUMP_STOP,
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

typedef enum job_state {
    FG,
    BG
} job_state;

typedef union {
    int dataI;
    float dataF;
} container;

typedef struct command_token {
    command_state command;
    job_state jstate;
    container argv[MAXARGS];
    int argc;
} command_token;

#endif
