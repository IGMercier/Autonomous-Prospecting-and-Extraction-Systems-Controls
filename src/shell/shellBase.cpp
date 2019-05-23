#include <thread>
#include <cstring>
#include <cstdlib>
#include <cstdio>
#include <unistd.h>
#include <string>
#include "shellBase.h"
//#include "../misc/flags.h"
//#include "../misc/flags_set.h"
#include "../misc/rio.h"

typedef struct thread_arg {
    char **argv;
    int bg;
    int len;
} thread_arg;

static void execute(thread_arg *arg);
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
    char *argv[MAXARGS];
    char buf[MAXLINE];
    int bg;
    thread child;

    strcpy(buf, cmdline);
    bg = parseline(buf, argv);

    if (argv[0] == NULL) {
        return;
    }

    if (!command(argv)) {
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

int ShellBase::command(char **argv) {
    if (!strcmp(argv[0], "quit")) {
        exit(0);
    }
    if (!strcmp(argv[0], "&")) {
        return 1;
    }

    return 0;

}

int ShellBase::parseline(char *buf, char **argv) {
    char *delim;
    int argc;
    int bg;

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

    if ((bg = (*argv[argc-1] == '&')) != 0) {
        argv[--argc] = NULL;
    }


    return bg;
}

void ShellBase::shell_print(std::string msg) {
    std::string toPrint = "Shell: " + msg;

    rio_writen(*(this->readFrom), toPrint.c_str(), strlen(msg));
    return;
}

void execute(thread_arg *arg) {
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
