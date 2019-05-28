#ifndef _SERVER_BASE_H_
#define _SERVER_BASE_H_

#define MAXCONN 1

class ServerBase {
    protected:
        int sfd;
        int cfd;
        void createServer(int port);
        int createClient();
        int setServerSockOpts();
        int setClientSockOpts();
        int checkSockOpts();
        virtual void execute();
        int readFromClient(char *cmdline);
        int sendToClient(const char *msg);

    public:
        ServerBase();
        virtual void shutdown();
        ~ServerBase();
};

#endif
