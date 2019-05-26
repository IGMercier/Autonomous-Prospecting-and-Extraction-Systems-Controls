#ifndef _APES_SERVER_H_
#define _APES_SERVER_H_

#include "serverBase.h"
#include <string>
#include <deque>

class APESServer : public ServerBase {
    private:
        std::deque<char *> *cmdq;
        std::deque<char *> *logq;
    public:
        APESServer(std::deque<char *> *cmdq, std::deque<char *> *logq);
        void run();
        void execute();
        void shutdown();
        ~APESServer();
};

#endif
