#ifndef _SERVER_H_
#define _SERVER_H_

#include "serverBase.h"
#include "commands.h"
#include <string>
#include <vector>

class APESServer : public ServerBase {
    public:
        APESServer();
        void run();
        void execute();
        void shutdown();
        ~APESServer();
};

#endif
