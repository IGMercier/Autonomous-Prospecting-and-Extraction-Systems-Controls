#include <thread>
#include <cstring>
#include <unistd.h>
#include "APESShell.h"
#include "../APESsys/commands.h"
#include "../misc/rio.h"

using std::thread;
static void execute(token *tk, int bg, int len, APESShell *shell);

APESShell::APESShell(APES *robot, int *readFrom):Shell(int *readFrom) {
    this->robot = robot;
}

void APESShell::evaluate(char *cmdline) {
    char *argv[MAXARGS];
    char buf[MAXLINE];
    int bg;
    thread child;
    token tk;

    strncpy(buf, cmdline, MAXLINE);
    bg = parseline(buf, argv, &tk);

    if (argv[0] == NULL) { return; }
    if (!command(argv, &tk)) {
        /* CHILD THREAD */
        thread temp(execute, &tk, bg, MAXLINE, this);
        child.swap(temp);

        if (bg) { child.detach(); }
    }

    /* PARENT THREAD */
    if (!bg) {
        if (child.joinable()) {
            child.join();
        }
    }
    return;
}

APESShell::~APESShell() {
    delete this->robot;
}

static void execute(token *tk, int bg, int len, APESShell *shell) {
    assert(tk != NULL);
    assert(shell != NULL);
    
    command_state command = tk->command;
    std::string msg;

    switch (command) {
        case START:
            if (VERBOSE) {
                msg = "System started!\n";
                shell->shell_print(msg);
            }
            break;
        case STANDBY:
            if (VERBOSE) {
                msg = "System in standby!\n";
                shell->shell_print(msg);
            }
            break;
        case DATA:
            if (VERBOSE) {
                msg = "Reading from data file!\n";
                shell->shell_print(msg);
            }
            break;
        case HELP:
            if (VERBOSE) {
                msg = "Listing Help Commands!\n";
                shell->shell_print(msg);
            }
            break;
        case QUIT:
            if (VERBOSE) {
                msg = "System shutting down!\n";
                shell->shell_print(msg);
            }
            break;
        case AUTO_ON:
            if (VERBOSE) {
                msg = "System's auto mode enabled!\n";
                shell->shell_print(msg);
            }
            break;
        case AUTO_OFF:
            if (VERBOSE) {
                msg = "System's auto mode disabled!\n";
                shell->shell_print(msg);
            }
            break;
        case TEMP:
            if (VERBOSE) {
                msg = "Reading temp!\n";
                shell->shell_print(msg);
            }
            break;
        case DTEMP:
            if (VERBOSE) {
                msg = "Reading dtemp!\n";
                shell->shell_print(msg);
            }
            break;
        case CURR:
            if (VERBOSE) {
                msg = "Reading curr!\n";
                shell->shell_print(msg);
            }
            break;
        case WLEVEL:
            if (VERBOSE) {
                msg = "Reading wlevel!\n";
                shell->shell_print(msg);
            }
            break;
        case WOB:
            if (VERBOSE) {
                msg = "Reading wob!\n";
                shell->shell_print(msg);
            }
            break;
        case MOTOR_DRIVE:
            if (VERBOSE) {
                msg = "System's motor enabled for []!\n";
                shell->shell_print(msg);
            }
            break;
        case MOTOR_STOP:
            if (VERBOSE) {
                msg = "System's motor disabled!\n";
                shell->shell_print(msg);
            }
            break;
        case DRILL_RUN:
            if (VERBOSE) {
                msg = "System's drill enabled!\n";
                shell->shell_print(msg);
            }
            break;
        case DRILL_STOP:
            if (VERBOSE) {
                msg = "System's drill disabled!\n";
                shell->shell_print(msg);
            }
            break;
        case DRILL_CYCLE:
            if (VERBOSE) {
                msg = "System's drill duty cycle changed!\n";
                shell->shell_print(msg);
            }
            break;
        case NONE:
        default:
            if (VERBOSE) {
                msg = "Not a valid command (use 'help' for more info)!\n";
                shell->shell_print(msg);
            }
            break;
    }
}
