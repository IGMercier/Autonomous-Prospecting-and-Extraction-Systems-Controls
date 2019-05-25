#ifndef _SERVER_H_
#define _SERVER_H_

#include "serverBase.h"
#include "commands.h"
#include <string>

class APESServer : public ServerBase {
    private:
        std::string cmdfile;
        std::string logfile;
    public:
        APESServer(std::string cmdfile, std::string logfile);
        void run();
        void execute();
        void shutdown();
        ~APESServer();
};

#endif
