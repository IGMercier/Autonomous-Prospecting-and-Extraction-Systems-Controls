#ifndef _APES_SHELL_H_
#define _APES_SHELL_H_
#include "shellBase.h"
//#include "../APESsys/APES.h"

#define VERBOSE 1

class APESShell : public ShellBase {
    protected:
        //APES *robot;
        void evaluate(char *cmdline);
    public:
        APESShell(/*APES *robot, */char *cmdfile, char *logfile) : ShellBase(cmdfile, logfile) {
            //this->robot = robot;    
        }
        void parsecommand(parse_token *ltk, command_token *ctk);
        ~APESShell();

};

#endif
