#ifndef _SERVER_BASE_H_
#define _SERVER_BASE_H_

#define MAXCONN 1

#include <string>

class ServerBase {
    public:
        int sfd;
        int cfd;

        ServerBase();
        void createServer(int port);
        int createClient();
        virtual void run();
        virtual void execute();
        int setServerSockOpts();
        int setClientSockOpts();
        int checkSockOpts();
        int readFromClient(char *cmdline);
        void sendToClient(const char *msg);
        virtual void shutdown();
        ~ServerBase();
};

#endif
