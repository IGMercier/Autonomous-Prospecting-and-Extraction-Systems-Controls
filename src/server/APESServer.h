#ifndef _APES_SERVER_H_
#define _APES_SERVER_H_

#include "serverBase.h"
#include "../misc/flags.h"

class APESServer : public ServerBase {
    private:
        std::mutex *cmd_mtx;
        std::mutex *log_mtx;
        std::deque<char *> *cmdq;
        std::deque<char *> *logq;
    public:
        APESServer(sysArgs *args);
        void run();
        void execute();
        void shutdown();
        ~APESServer();
};

#endif
