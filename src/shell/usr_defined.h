#ifndef _USR_DEFINED_H_
#define _USR_DEFINED_H_

typedef struct thread_arg {
    char **argv;
    int bg;
    int len;
} thread_arg;

void execute(thread_arg *arg);

#endif
