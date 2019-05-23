#include "usr_defined.h"
#include <cstdlib>
#include <cstdio>

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
