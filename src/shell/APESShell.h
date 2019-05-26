#ifndef _APES_SHELL_H_
#define _APES_SHELL_H_
#include "shellBase.h"
//#include "../APESsys/APES.h"

#define VERBOSE 1
#include <string>
#include <deque>

class APESShell : public ShellBase {
    protected:
        //APES *robot;
    public:
        std::deque<char *> *cmdq;
        std::deque<char *> *logq;
        APESShell(std::deque<char *> *cmdq, std::deque<char *> *logq);
        void run() override;
        void toSend(std::string msg);
        void evaluate(char *cmdline) override;
        void parsecommand(parse_token *ltk, command_token *ctk);
        ~APESShell();

};

#endif
