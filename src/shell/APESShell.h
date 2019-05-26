#ifndef _APES_SHELL_H_
#define _APES_SHELL_H_
#include "shellBase.h"
//#include "../APESsys/APES.h"

#define VERBOSE 1
#include <string>

class APESShell : public ShellBase {
    protected:
        //APES *robot;
    public:
        std::string cmdfile;
        std::string logfile;
        APESShell(std::string cmdfile, std::string logfile);
        void run() override;
        void toSend(std::string msg);
        void evaluate(char *cmdline) override;
        void parsecommand(parse_token *ltk, command_token *ctk);
        ~APESShell();

};

#endif
