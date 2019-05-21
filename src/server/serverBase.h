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

/*
    NEWTORK-SAFE FILE READ/WRITE (BUFFERED)
    from Computer Systems by Bryant & O'Hallaron
*/
#define RIO_BUFSIZE 8192
typedef struct {
    int rio_fd;                /* Descriptor for this internal buf */
    ssize_t rio_cnt;           /* Unread bytes in internal buf */
    char *rio_bufptr;          /* Next unread byte in internal buf */
    char rio_buf[RIO_BUFSIZE]; /* Internal buffer */
} rio_t;

static ssize_t rio_read(rio_t *rp, char *usrbuf, size_t n);
ssize_t rio_readn(int fd, void *usrbuf, size_t n);
ssize_t rio_writen(int fd, void *usrbuf, size_t n);
void rio_readinitb(rio_t *rp, int fd);
ssize_t	rio_readnb(rio_t *rp, void *usrbuf, size_t n);
ssize_t	rio_readlineb(rio_t *rp, void *usrbuf, size_t maxlen);

#endif
