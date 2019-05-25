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
        int readFromClient(char *cmdline, int len);
        void sendToClient(std::string msg);
        void shutdown();
        ~ServerBase();
};

#endif
