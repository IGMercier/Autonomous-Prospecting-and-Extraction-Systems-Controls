#ifndef _SERVER_BASE_H_
#define _SERVER_BASE_H_

#include <pthread.h>

class ServerBase {
    public:
        ServerBase();
        int sfd;
        int cfd;
        void serverSetup(int port);
        void clientSetup();
        int createClient(pthread_t tid, int flags);
        static void *thread(void *arg);
        void readFromClient(char *cmdline);
        void sendToClient(const char *msg);
        void shutdown();
        ~ServerBase();
};

#endif
