#ifndef _APES_SERVER_H_
#define _APES_SERVER_H_

#include <string>
#include <vector>
#include "serverBase.h"

class APESServer : public ServerBase {
    public:
        char *cmdfile;
        char *logfile;
        APESServer(char *cmdfile, char *logfile);
        void run();
        void shutdown();
        ~APESServer();
};

#endif
