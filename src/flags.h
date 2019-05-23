#ifndef _FLAGS_H_
#define _FLAGS_H_

#include <signal.h>
sig_atomic_t disconnected = 1;
sig_atomic_t shutdownSIG = 0;

typedef struct thread_arg {
    char **argv;
    int bg;
    int len;
} thread_arg;

void execute(thread_arg *arg);


#endif
