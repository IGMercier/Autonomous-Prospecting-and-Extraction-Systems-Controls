#ifndef _SERVER_H_
#define _SERVER_H_

#include "serverBase.h"
#include "commands.h"
#include <string>

class APESServer : public ServerBase {
    private:
        std::string cmdfile;
    public:
        APESServer(std::string cmdfile);
        void run();
        void execute();
        void shutdown();
        ~APESServer();
};

#endif
