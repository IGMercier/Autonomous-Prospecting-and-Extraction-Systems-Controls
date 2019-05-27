#ifndef _APES_SERVER_H_
#define _APES_SERVER_H_

#include "serverBase.h"
#include "../misc/flags.h"
#include <string>

class APESServer : public ServerBase {
    private:
        std::mutex *cmd_mtx;
        std::mutex *log_mtx;
        std::deque<std::string> *cmdq;
        std::deque<std::string> *logq;
    public:
        APESServer(sysArgs *args);
        void run(int port) override;
        void execute() override;
        void shutdown() override;
        virtual ~APESServer();
};

#endif
