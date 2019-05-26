#include <cstdlib>
#include <cstring>
#include <cstdio>
#include <unistd.h>
#include <thread>
#include <assert.h>
#include "shellBase.h"
#include <assert.h>
//#include "../misc/flags.h"
#include "../misc/rio.h"

static void execute(parse_token *tk, int bg);

using std::thread;
ShellBase::ShellBase() {}
ShellBase::~ShellBase() {}

void ShellBase::run() {
    char cmdline[MAXLINE];
    rio_t buf;
    
    while (1) {
        rio_readinitb(&buf, STDIN_FILENO);
        rio_readlineb(&buf, cmdline, MAXLINE);

        if (feof(stdin)) {
            printf("\n");
            fflush(stdout);
            fflush(stdin);
        }
        evaluate(cmdline);
    }
    return; // kills shell thread in main program
}

void ShellBase::evaluate(char *cmdline) {
    int bg;
    parse_token tk;
    thread child;

    bg = parseline(cmdline, &tk);

    if (tk.argc == 0) {
        return;
    }

    if (!builtin_command(&tk)) {
        /* CHILD THREAD */
        thread temp(execute, &tk, bg);
        child.swap(temp);

        if (bg) {
            child.detach();
        }
    }

    /* PARENT THREAD */
    if (!bg) {
        if (child.joinable()) { // safeguard against <builtin> &
            // wait for child thread to end
            child.join();
        }
    }

    return;
}

// returns 1 if a builtin, 0 otherwise
int ShellBase::builtin_command(parse_token *tk) {
    assert(tk != NULL);

    builtin bcomm = tk->bcomm;

    switch(bcomm) {
        case BUILTIN_FG:
            return 1;
        case BUILTIN_BG:
            return 1;
        case BUILTIN_JOBS:
            return 1;
        case BUILTIN_NONE:
        default:
            return 0;
    }
}

int ShellBase::parseline(char *cmdline, parse_token *tk) {
    char *delim = " \t\n";
    char *argv;
    int bg;

    int argc = 0;
    tk->argc = 0;

    while ((argv = strsep(&cmdline, delim)) != NULL) {
        tk->argv[argc] = argv;
        argc++;
        if (argc == MAXARGS) { break; } 
    }

    tk->argc = argc;

    if (tk->argc == 0) { return 1; }

    
    if (!strcmp(tk->argv[0], "fg")) {
        tk->bcomm = BUILTIN_FG;
    } else if (!strcmp(tk->argv[0], "bg")) {
        tk->bcomm = BUILTIN_BG;
    } else if (!strcmp(tk->argv[0], "jobs")) {
        tk->bcomm = BUILTIN_JOBS;
    } else {
        tk->bcomm = BUILTIN_NONE;
    }

    if ((bg = (*(tk->argv[(tk->argc)-2]) == '&')) != 0) {
        tk->argv[--(tk->argc)] = NULL;
        tk->argv[--(tk->argc)] = NULL;
    }

    return bg;
}

void ShellBase::print(std::string msg) {
    std::string toPrint = "Shell: " + msg;
    printf(msg.c_str());
    return;
}

static void execute(parse_token *tk, int bg) {
    assert(tk != NULL);
    fprintf(stdout, "IN THREAD\n");

    if (bg) {
        fprintf(stdout, "\t\t\tA BG JOB\n");
    }

    for (int i = 0; i < tk->argc; i++) {
        if (tk->argv[i] == NULL) { break; }
        fprintf(stdout, "%s ", tk->argv[i]);
    }

    if (bg) {
        fprintf(stdout, "\n");
        
        fflush(stdout);
        
    }
    return;
}
