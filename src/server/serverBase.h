#ifndef _SERVER_BASE_H_
#define _SERVER_BASE_H_

#define MAXCONN 1

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
        int createClient();
        int readFromClient(char *cmdline);
        void sendToClient(const char *msg);
        void shutdown();
        ~ServerBase();
};

#endif
