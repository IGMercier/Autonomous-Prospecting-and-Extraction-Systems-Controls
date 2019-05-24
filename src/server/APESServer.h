#ifndef _APES_SERVER_H_
#define _APES_SERVER_H_

#include <string>
#include <vector>
#include "serverBase.h"

class APESServer : public ServerBase {
    private:
    public:
        APESServer();
        void run(int *shell_cfd);
        void shutdown();
        ~APESServer();
};

#endif
