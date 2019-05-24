#ifndef _SHELL_BASE_H_
#define _SHELL_BASE_H_

// extern char **environ;

#include <string>
#include "../APESsys/commands.h"

typedef struct parse_token {
    int argc;
    char *argv[MAXARGS];
    builtin bcomm;
} parse_token;

class ShellBase {
    protected:
        int *readFrom;
        virtual void evaluate(char *cmdline);
        int parseline(char *cmdline, parse_token *tk);
        int builtin_command(parse_token *tk);
    public:
        ShellBase(int *readFrom);
        void run();
        void shell_print(std::string msg);
        ~ShellBase();
};


#endif
