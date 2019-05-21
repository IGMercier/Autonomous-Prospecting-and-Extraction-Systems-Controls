#ifndef _SERVER_H_
#define _SERVER_H_

#include "serverBase.h"
#include "commands.h"
#include <string>
#include <vector>

class Server : public ServerBase {
    private:
        std::vector<std::string> commandList;
    public:
        Server();
        void clientSetup();
        static void *thread(void *arg);
        int command(token *tk);
        void setCommands();
        void listCommands();
        void shutdown();
        ~Server();
};

#endif
