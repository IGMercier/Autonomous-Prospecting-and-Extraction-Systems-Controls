#ifndef _SERVER_H
#define _SERVER_H

#include "serverBase.h"
#include <string>
#include <vector>

class Server::public ServerBase {
    private:
        std::vector<std::string> commandList;
    public:
        Server();
        void *thread(void *arg);
        int command(token *tk);
        void setCommands();
        void listCommands();
        void shutdown();
        ~Server();
}

#endif
