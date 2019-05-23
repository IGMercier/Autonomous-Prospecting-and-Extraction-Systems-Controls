#include "flags.h"

void execute(thread_arg *arg) {
    fprintf(stdout, "IN THREAD\n");

    if (arg->bg) {
        fprintf(stdout, "A BG JOB\n");
    }

    for (int i = 0; i < arg->len; i++) {
        if (arg->argv[i] == NULL) { break; }

        printf("%s\n", arg->argv[i]);
    }
    return;
}
