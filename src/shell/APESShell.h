#ifndef _APES_SHELL_H_
#define _APES_SHELL_H_

#include "shellBase.h"
#include "../misc/flags.h"
//#include "../APESsys/APES.h"

#include <string>

class APESShell : public ShellBase {
    public:
        std::mutex *cmd_mtx;
        std::mutex *log_mtx;
        std::deque<std::string> *cmdq;
        std::deque<std::string> *logq;

        APESShell(sysArgs *args);
        void run() override;
        void evaluate(std::string cmdline) override;
        void parsecommand(parse_token *ltk, command_token *ctk);
        virtual ~APESShell();

};

#endif
