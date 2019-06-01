#ifndef _SERVER_BASE_H_
#define _SERVER_BASE_H_

#define MAXCONN 1

#include <string>

class ServerBase {
    protected:
        int sfd;
        volatile int cfd;
        void createServer(int port);
        int createClient();
        int setServerSockOpts();
        int setClientSockOpts();
        int checkSockOpts();
        virtual void execute();
        int readFromClient(char *cmdline);
        int sendToClient(std::string msg);
        void print(std::string msg);

    public:
        ServerBase();
        virtual void shutdown();
        ~ServerBase();
};

#endif
