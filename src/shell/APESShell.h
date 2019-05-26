#ifndef _APES_SHELL_H_
#define _APES_SHELL_H_

#include "shellBase.h"
#include "../misc/flags.h"
//#include "../APESsys/APES.h"

#define VERBOSE 1
#include <string>

class APESShell : public ShellBase {
    public:
        std::mutex *cmd_mtx;
        std::mutex *log_mtx;
        std::deque<char *> *cmdq;
        std::deque<char *> *logq;

        APESShell(sysArgs *args);
        void run() override;
        void evaluate(char *cmdline) override;
        void parsecommand(parse_token *ltk, command_token *ctk);
        void toSend(std::string msg);
        ~APESShell();

};

#endif
