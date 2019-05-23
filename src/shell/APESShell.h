#ifndef _APES_SHELL_H_
#define _APES_SHELL_H_
#include "shellBase.h"
#include "../APESsys/APES.h"

class APESShell : public ShellBase {
    private:
        APES *robot;
    public:
        APESShell(APES *robot) : ShellBase(int *shell_cfd);
        void run();
        ~APESShell();

};

#endif
