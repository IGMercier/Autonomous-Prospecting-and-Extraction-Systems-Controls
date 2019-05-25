#ifndef _SERVER_BASE_H_
#define _SERVER_BASE_H_

#define MAXCONN 1

#include <string>

class ServerBase {
    public:
        ServerBase();
        int sfd;
        int cfd;
        void createServer(int port);
        int setServerSockOpts();
        int setClientSockOpts();
        int checkSockOpts();
        void run();
        void connection();
        int createClient();
        static void *thread(void *arg);
        int readFromClient(char *cmdline);
        void sendToClient(const char *msg);
        void shutdown();
        ~ServerBase();
};

#endif
