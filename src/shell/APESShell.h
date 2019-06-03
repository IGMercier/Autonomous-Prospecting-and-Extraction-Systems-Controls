#ifndef _APES_SHELL_H_
#define _APES_SHELL_H_

#include "shellBase.h"
#include "../misc/flags.h"
#include "../APESsys/APES.h"

#include <string>

class APESShell : public ShellBase {
    private:
        APES *robot;
    public:
        std::mutex *cmd_mtx;
        std::mutex *log_mtx;
        std::deque<std::string> *cmdq;
        std::deque<std::string> *logq;

        APESShell(sysArgs *args);
        void run() override;
        void execute(parse_token *ltk);
        void listCommands();
        void evaluate(std::string cmdline) override;
        void parsecommand(parse_token *ltk, command_token *ctk);
        void toSend(std::string msg);
        void shutdown();
	    virtual ~APESShell();

};

#endif
