#ifndef _SERVER_H_
#define _SERVER_H_

#include "serverBase.h"
#include "commands.h"
#include <string>
#include <vector>

class Server : public ServerBase {
    private:
    public:
        Server();
        void run(int *shell_cfd);
        void shutdown();
        ~Server();
};

#endif
