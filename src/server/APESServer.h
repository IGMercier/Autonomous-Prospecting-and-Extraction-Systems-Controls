#ifndef _APES_SERVER_H_
#define _APES_SERVER_H_

#include "serverBase.h"
#include "../misc/flags.h"
#include <string>

class APESServer : public ServerBase {
    private:
        std::string datafile;
        std::mutex *cmd_mtx = nullptr;
        std::mutex *log_mtx = nullptr;
        std::mutex *data_mtx = nullptr;
        std::deque<std::string> *cmdq = nullptr;
        std::deque<std::string> *logq = nullptr;
        void execute() override;
        void disconnected();
    public:
        APESServer(sysArgs *args);
        void run(int port);
        void shutdown() override;
        virtual ~APESServer();
};

#endif
