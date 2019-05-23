#include "shell.h"

#define MAXLINE 1024
#define MAXARGS 128

#include <thread>
#include <cstring>
#include <cstdlib>
#include <cstdio>
#include <unistd.h>

using std::thread;

// put these in external header

typedef struct thread_arg {
    char **argv;
    int bg;
} thread_arg;

void execute(thread_arg *arg) {
    fprintf(stdout, "IN THREAD\n");

    if (arg->bg) {
        fprintf(stdout, "A BG JOB\n");
    }

    for (int i = 0; i < MAXARGS; i++) {
        if (arg->argv[i] == NULL) { break; }

        printf("%s\n", arg->argv[i]);
    }
    return;
}
//

Shell::Shell() {}
Shell::~Shell() {}

void Shell::run() {
    char cmdline[MAXLINE];

    while (1) {
        fgets(cmdline, MAXLINE, stdin);
        if (feof(stdin)) {
            printf("\n");
            fflush(stdout);
            fflush(stdin);
        }

        evaluate(cmdline);
    }
    return;
}

void Shell::evaluate(char *cmdline) {
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

int Shell::command(char **argv) {
    if (!strcmp(argv[0], "quit")) {
        exit(0);
    }
    if (!strcmp(argv[0], "&")) {
        return 1;
    }

    return 0;

}

int Shell::parseline(char *buf, char **argv) {
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
