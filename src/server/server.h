#ifndef _SERVER_H
#define _SERVER_H

#include "serverBase.h"

class Server::public ServerBase {
    public:
        Server();
        void *thread(void *arg);
        int command(token *tk);
        void shutdown();
        ~Server();
}

#endif
