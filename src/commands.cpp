#include "commands.h"
#include <string>

using std::string;

int parseline(const char *cmdline, token *tk) {
    const char delims[] = " \t\r\n";
    char *buf;
    char *next;
    char *endbuf;

    if (cmdline == NULL) {
        return -1;
    }

    strncpy(tk->text, cmdline, MAXLINE);

    buf = tk->text;
    endbuf = tk->text + strlen(tk->text);

    tk->argc = 0;

    while (buf < endbuf) {
        buf += strspn(buf, delims); // skips by num of delimiters
        if (buf >= endbuf) { break; }

        next = buf + strcspn(buf, delims);

        *next = '\0'; // make buf into valid c string

        tk->argv[tk->argc] = buf;
        tk->argc = tk->argc + 1;

        if (tk->argc >= MAXARGS - 1) { break; }

        buf = next + 1;
    }

    tk->argv[tk->argc] = NULL;
    if (tk->argc == 0) { return -1; }

    if ((strcmp(tk->argv[0], "quit")) == 0) {
        tk->command = QUIT;
    } else if ((strcmp(tk->argv[0], "start")) == 0) {
        tk->command = START;
    } else if ((strcmp(tk->argv[0], "standby")) == 0) {
        tk->command = STANDBY;
    } else if ((strcmp(tk->argv[0], "temp")) == 0) {
        tk->command = TEMP;
    } else if ((strcmp(tk->argv[0], "dtemp")) == 0) {
        tk->command = DTEMP;
    } else if ((strcmp(tk->argv[0], "curr")) == 0) {
        tk->command = CURR;
    } else if ((strcmp(tk->argv[0], "level")) == 0) {
        tk->command = LEVEL;
    } else if ((strcmp(tk->argv[0], "wob")) == 0) {
        tk->command = WOB;
    } else if ((strcmp(tk->argv[0], "motor_drive")) == 0) {
        tk->command = MOTOR_DRIVE;
    } else if ((strcmp(tk->argv[0], "motor_stop")) == 0) {
        tk->command = MOTOR_STOP;
    } else if ((strcmp(tk->argv[0], "drill_run")) == 0) {
        tk->command = DRILL_RUN;
    } else if ((strcmp(tk->argv[0], "drill_stop")) == 0) {
        tk->command = DRILL_STOP;
    } else if ((strcmp(tk->argv[0], "drill_cycle")) == 0) {
        tk->command = DRILL_CYCLE;
    } else if ((strcmp(tk->argv[0], "auto")) == 0) {
        tk->command = AUTO;
    } else if ((strcmp(tk->argv[0], "disconnected")) == 0) {
        tk->command = DISCONNECTED;
    } else {
        tk->command = NONE;
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
