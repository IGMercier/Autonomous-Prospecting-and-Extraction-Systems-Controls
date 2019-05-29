#ifndef _SHELL_BASE_H_
#define _SHELL_BASE_H_

#include <string>

#include "../APESsys/commands.h"

typedef struct parse_token {
    int argc;
    std::string argv[MAXARGS];
    builtin bcomm;
} parse_token;

class ShellBase {
    protected:
        virtual void evaluate(std::string cmdline);
        int parseline(std::string cmdline, parse_token *tk);
        int builtin_command(parse_token *tk);
    public:
        ShellBase();
        virtual void run();
        void print(std::string msg);
        virtual ~ShellBase();
};


#endif
