#ifndef _SHELL_BASE_H_
#define _SHELL_BASE_H_

extern char **environ;

#include <string>
#include "../APESsys/commands.h"

class ShellBase {
    protected:
        int *readFrom;
        void evaluate(char *cmdline);
        int parseline(char *cmdline, parse_token *tk);
        int builtin_command(parse_token *tk);
        void shell_print(std::string msg);
    public:
        ShellBase(int *readFrom);
        void run();
        ~ShellBase();
};


#endif
