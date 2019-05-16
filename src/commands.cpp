#include "commands.h"
#include <string>

int parseline(const char *cmdline, token *tk) {
    const char delims[] = " \t\r\n";
    char *buf;
    char *next;
    char *endbuf;

    if (cmdline == NULL) {
        return -1;
    }

    strncopy(token->text, cmdline, MAXLINE_TSH);

    buf = token->text;
    endbuf = token->text + strlen(token->text);

    token->argc = 0;

    while (buf < endbuf) {
        buf += strspn(buf, delims);
        if (buf >= endbuf) { break; }

        next = buf + strcspn(buf, delims);

        *next = '\0';

        token->argv[token->argc] = buf;
        token->argc = token->argc + 1;

        if (token->argc >= MAXARGS - 1) { break; }

        buf = next + 1;
    }
    token->argv[token->argc] = NULL;
    if (token->argc == 0) { return -1; }

    if ((strcmp(token->argv[0], "quit")) == 0) {
        token->command_state = QUIT;
    } else if ((strcmp(token->argv[0], "start")) == 0) {
        token->command_state = START;
    } else if ((strcmp(token->argv[0], "standby")) == 0) {
        token->command_state = STANDBY;
    } else if ((strcmp(token->argv[0], "temp")) == 0) {
        token->command_state = TEMP;
    } if ((strcmp(token->argv[0], "dtemp")) == 0) {
        token->command_state = DTEMP;
    } else if ((strcmp(token->argv[0], "curr")) == 0) {
        token->command_state = CURR;
    } else if ((strcmp(token->argv[0], "level")) == 0) {
        token->command_state = LEVEL;
    } else if ((strcmp(token->argv[0], "wob")) == 0) {
        token->command_state = QUIT;
    } else if ((strcmp(token->argv[0], "motor_drive")) == 0) {
        token->command_state = MOTOR_DRIVE;
    } else if ((strcmp(token->argv[0], "motor_stop")) == 0) {
        token->command_state = MOTOR_STOP;
    } else if ((strcmp(token->argv[0], "drill_run")) == 0) {
        token->command_state = DRILL_RUN;
    } else if ((strcmp(token->argv[0], "drill_stop")) == 0) {
        token->command_state = DRILL_STOP;
    } else if ((strcmp(token->argv[0], "drill_cycle")) == 0) {
        token->command_state = DRILL_CYCLE;
    } else if ((strcmp(token->argv[0], "auto")) == 0) {
        token->command_state = AUTO;
    } else {
        token->command_state = NONE;
    }

    return 0;
}

const char* listCommands() {
    string msg = "Help - Commands:\n";
    msg += "help - prints this message\n";
    msg += "start - initializes system\n";
    msg += "standby - stops all components, system in standby\n";
    msg += "temp - returns current temperature reading\n";
    msg += "dtemp - returns temperature difference from startup\n";
    msg += "curr - returns current current reading\n";
    msg += "level - returns current level reading\n";
    msg += "wob - returns current force reading\n";
    msg += "data - returns the data file\n";
    msg += "motor_drive - drive the motor\n";
    msg += "motor_stop - stops the motor\n";
    msg += "drill_run - runs the drill\n";
    msg += "drill_stop - stops the drill\n";
    msg += "drill_cycle - changes drill pwm\n";
    msg += "auto - puts system in auto mode\n";
    msg += "quit - shutdown the entire system, including server\n";
    return msg.c_str();
}
