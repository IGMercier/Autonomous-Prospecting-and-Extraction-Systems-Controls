#include <thread>
#include <cstring>
#include <cstdlib>
#include <cstdio>
#include <unistd.h>
#include <string>
#include "shellBase.h"
#include "../APESsys/commands.h"
//#include "../misc/flags.h"
//#include "../misc/flags_set.h"
#include "../misc/rio.h"

static void execute();
int shutdownSIG = 0; // remove after testing!!!

using std::thread;

ShellBase::ShellBase(int *readFrom) {
    if (readFrom == NULL) {
        this->readFrom = STDIN_FILENO;
    } else {
        this->readFrom = readFrom;
    }
}

ShellBase::~ShellBase() {}

void ShellBase::run() {
    char cmdline[MAXLINE];

    rio_t buf;
    rio_readinitb(&buf, *(this->readFrom));

    while (!shutdownSIG) {
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
    token tk;
    thread child;

    bg = parseline(cmdline, &tk);

    if (tk.argv[0] == NULL) {
        return;
    }

    if (!builtin(&tk)) {
        /* CHILD THREAD */
        thread_arg arg;
        arg.argv = argv;
        arg.bg = bg;
        arg.len = MAXLINE;
        thread temp(execute, &arg);
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
int ShellBase::builtin(token *tk) {
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

int ShellBase::parseline(char *cmdline, token *tk) {
    char buf[MAXLINE];
    char *argv[MAXARGS];
    char *delim;
    int argc;
    int bg;

    strncpy(buf, cmdline, MAXLINE);

    buf[strlen(buf)-1] = ' ';

    while (*buf && (*buf == ' ')) { buf++; }

    argc = 0;
    while ((delim = strchr(buf, ' '))) {
        argv[argc++] = buf;
        *delim = '\0';
        buf = delim + 1;
        while (*buf && (*buf == ' ')) { buf++; }
    }

    argv[argc] = NULL;


    if (argc == 0) { return 1; }


    tk->argc = argc;
    tk->argv = argv;

    if (!strcmp(tk->argv[0], "fg")) {
        tk->bcomm = BUILTIN_FG;
    } else if (!strcmp(tk->argv[0], "bg")) {
        tk->bcomm = BUILTIN_BG;
    } else if (!strcmp(tk->argv[0], "jobs")) {
        tk->bcomm = BUILTIN_JOBS;
    } else {
        tk->bcomm = BUILTIN_NONE;
    }

    if ((bg = (*tk->argv[tk->argc-1] == '&')) != 0) {
        tk->argv[--(tk->argc)] = NULL;
    }

    return bg;
}

void ShellBase::shell_print(std::string msg) {
    std::string toPrint = "Shell: " + msg;

    rio_writen(*(this->readFrom), toPrint.c_str(), strlen(msg));
    return;
}

void execute(token *tk) {
    fprintf(stdout, "IN THREAD\n");

    if (arg->bg) {
        fprintf(stdout, "\t\t\tA BG JOB\n");
    }

    for (int i = 0; i < arg->len; i++) {
        if (arg->argv[i] == NULL) { break; }

        printf("%s ", arg->argv[i]);
    }
    printf("\n");
    return;
}
