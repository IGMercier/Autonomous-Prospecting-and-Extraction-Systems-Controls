#ifndef _APES_SERVER_H_
#define _APES_SERVER_H_

#include "serverBase.h"
#include "commands.h"
#include <string>
#include <vector>

class APESServer : public ServerBase {
    private:
    public:
        APESServer();
        void run(int *shell_cfd);
        void shutdown();
        ~APESServer();
};

#endif
