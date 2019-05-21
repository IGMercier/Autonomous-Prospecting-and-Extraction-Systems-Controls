#ifndef _SERVER_BASE_H_
#define _SERVER_BASE_H_

class ServerBase {
    protected:
        int server_fd;
        int client_fd;
    public:
        ServerBase();
        void serverSetup(int port);
        void clientSetup();
        void readFromClient(char *cmdline);
        void sendToClient(const char *msg);
        void* thread(void *arg);
        void shutdown();
        ~ServerBase();
};

#endif
