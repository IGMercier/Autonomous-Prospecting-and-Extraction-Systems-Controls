/*
    To test out server only, comment out robot functions and run:
    g++ -o server server.cpp commands.cpp -lpthread
    ./server <port number>
*/

#include "commands.h"
#include <cstdlib>
#include <cstdio>
#include <string>
#include <cstring>
#include <csignal>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/tcp.h>
#include <errno.h>
#include <assert.h>
#include <pthread.h>
#include "APES.h"

static int server_fd;
static volatile int disconnected = 1;
static volatile int shutdownSIG = 0;
APES robot;

int serverSetup(int port);
void  clientSetup();
static int readFromClient(int client_fd, char *cmdline);
static int sendToClient(int client_fd, const char *msg);
static int command(int *client_fd, token *tk);
static void listCommands(int client_fd);
static void shutdown(int client_fd);
static void *thread(void *arg);

static void sigint_handler(int sig);
static void sigpipe_handler(int sig);


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

int main(int argc, char** argv) {
    /* @TODO:
       [] setup robot
       [] put robot in standby
       [] when connected:
         [X] read commands from client
         [] add commands to job list
	     [] execute commands from job list
       [X] when disconnected:
         [] put robot in standby
     */

    signal(SIGINT, sigint_handler); 
    signal(SIGPIPE, sigpipe_handler); 

    int port;
    if (argc < 2) {
        port = atoi("16778");
    } else {
        port = atoi(argv[1]);
    }

    server_fd = -1;
    fprintf(stdout, "Server starting up...!\n");
    while ((server_fd = serverSetup(port)) < 0) {
        fprintf(stderr, "ERROR: %s\n", strerror(errno));
        fprintf(stdout, "Retrying...\n");
        
        // retry to setup
    }

    assert(server_fd >= 0);
    clientSetup();

    // control should never reach here
    shutdown(-1);
    return -1;
}

/*
    SERVER/CLIENT FUNCTIONS
*/
int serverSetup(int port) {
    assert(port > 0);

    int flags = 1;
    struct sockaddr_in serveraddr;
    int sfd;

    if ((sfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        return -1;
    }

    // these if-statements setup the handshake to keep
    // alive the connection to client
    // if client does not respond, SIGPIPE is issued
    if (setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR,
                   (const void*)&flags, sizeof(int)) < 0) {
        close(sfd);
        return -1;
    }

    flags = 5;
    if (setsockopt(sfd, SOL_TCP, TCP_KEEPIDLE,
                   (const void*)&flags, sizeof(int)) < 0) {
        close(sfd);
        return -1;
    }
    flags = 2;
    if (setsockopt(sfd, SOL_TCP, TCP_KEEPCNT,
                   (const void*)&flags, sizeof(int)) < 0) {
        close(sfd);
        return -1;
    }
    flags = 5;
    if (setsockopt(sfd, SOL_TCP, TCP_KEEPINTVL,
                   (const void*)&flags, sizeof(int)) < 0) {
        close(sfd);
        return -1;
    }

    memset(&serveraddr, 0, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
    serveraddr.sin_port = htons((unsigned short)port);

    if (bind(sfd, (struct sockaddr *)&serveraddr, sizeof(serveraddr)) < 0) {
        close(sfd);
        return -1;
    }

    // allows only 1 connection before it starts rejecting connections
    if (listen(sfd, 1) < 0) {
        close(sfd);
        return -1;
    }

    return sfd;

}

void clientSetup() {
    pthread_t tid;
    int flags = 1;
    while (1) {
        int client_fd = -1;

        struct sockaddr_in clientaddr;
        socklen_t client_addr_size;

        client_addr_size = sizeof(struct sockaddr_in);
        // accept() blocks until client connects
        client_fd = accept(server_fd,
                        (struct sockaddr *)&clientaddr.sin_addr.s_addr,
                         &client_addr_size);
        if (client_fd < 0) {
            continue;
        }
        
        flags = 1;
        if (setsockopt(client_fd, SOL_SOCKET, SO_KEEPALIVE,
                       (const void*)&flags, sizeof(int)) < 0) {
            close(client_fd);
            continue;
        }

        std::string msg = "Connected...!\n";
        sendToClient(client_fd, msg.c_str());
        fprintf(stdout, "%s", msg.c_str());

        if (pthread_create(&tid, NULL, thread, (void *)(long)client_fd) < 0) {
            close(client_fd);
            continue;
        }

    }

    return;
}

static void *thread(void *arg) {
    assert(arg != NULL);

    pthread_detach(pthread_self());
    
    disconnected = 0;

    int client_fd = (int)(long)arg;

    while (!disconnected) {
        if (shutdownSIG) { shutdown(client_fd); }

        char *cmdline = (char *)calloc(MAXLINE, sizeof(char));
        token tk;

        readFromClient(client_fd, cmdline);
        parseline(cmdline, &tk);
        command(&client_fd, &tk);

        free(cmdline);

    }

    robot.standby();
    close(client_fd);
    fprintf(stdout, "Disconnected...!\n");
    return NULL;

}

/*
    EVALUATION FUNCTIONS
*/
static int readFromClient(int client_fd, char* cmdline) {
    assert(client_fd >= 0);
    assert(cmdline != NULL);

    rio_t buf;
    rio_readinitb(&buf, client_fd);
    rio_readlineb(&buf, cmdline, MAXLINE);
    
    cmdline[strlen(cmdline)-1] = '\0';

    return 0;
}

static int sendToClient(int client_fd, const char *msg) {
    assert(client_fd >= 0);
    assert(msg != NULL);

    rio_writen(client_fd, (void *)msg, strlen(msg));
    return 0;
}

static void shutdown(int client_fd) {
    assert(client_fd >= 0);
    robot.finish();

    std::string msg = "Shutting down!\n";
    sendToClient(client_fd, msg.c_str());
    fprintf(stdout, "%s", msg.c_str());

    if (close(client_fd) < 0) {
        fprintf(stderr, "ERROR: %s\n", strerror(errno));
    }
    if (close(server_fd) < 0) {
        fprintf(stderr, "ERROR: %s\n", strerror(errno));
    }
    exit(0);
}

static int command(int *client_fd, token *tk) {
    assert(tk != NULL);

    command_state command = tk->command;
    std::string msg;

    switch (command) {
        case START:
            // setup robot and retry on fail
            /*while (robot.setup("data.csv") < 0) {
                fprintf(stderr, "ERROR: APES system setup failure!\n");
                fprintf(stdout, "Retrying...\n");
                robot.finish();
            }
            */

            // put in standby
            robot.standby();
            msg = "System started!\n";
            sendToClient(*client_fd, msg.c_str());
            return 1;
        case HELP:
            //string helpLines[] = listCommands();
            //int size = *(&helpLines + 1) - helpLines;
            //for (int i = 0; i < size; i++) {
            //    sendToClient(*client_fd, helpLines[i].c_str());
            //}
            listCommands(*client_fd);
            return 1;
        case QUIT:
            // shuts down everything
            shutdown(*client_fd);
            return 1;
        case AUTO:
            // runs things automatically
            /*
                @TODO:
                Will this need to create a new thread?
                We want this to run in the background,
                but we also want to be able to input more
                commands...
                If we spawn a process, APES system will be copied,
                so we dont want that...
            */
            msg = "System in auto mode!\n";
            sendToClient(*client_fd, msg.c_str());
            return 1;
        case TEMP:
            /*
            float temp;
            temp = robot.read_temp();
            printf(stdout, "Temp (@time): %f\n", temp);
            */
            msg = "Temp (@time): \n";
            sendToClient(*client_fd, msg.c_str());
            return 1;
        case DTEMP:
            /*
            float dtemp;
            dtemp = robot.D_temp();
            */
            msg = "Temp since init: \n";
            sendToClient(*client_fd, msg.c_str());
            return 1;
        case CURR:
            /*
            float curr;
            curr = robot.read_curr();
            printf(stdout, "Curr (@time): %f\n", curr);
            */
            msg = "Curr (@time): \n";
            sendToClient(*client_fd, msg.c_str());
            return 1;
        case WLEVEL:
            /*
            int level;
            level = robot.read_level();
            printf(stdout, "Level (@time): %d\n", level);
            */
            msg = "Level (@time): \n";
            sendToClient(*client_fd, msg.c_str());
            return 1;
        case STANDBY:
            robot.standby();
            msg = "System in standby!\n";
            sendToClient(*client_fd, msg.c_str());
            return 1;
        case WOB:
            /*
            float force;
            force = robot.read_wob();
            printf(stdout, "Force (@time): %f\n", force);
            */
            msg = "Force (@time): \n";
            sendToClient(*client_fd, msg.c_str());
            return 1;
        case DATA:
            //robot.read_data();
            msg = "Data dump...\n";
            sendToClient(*client_fd, msg.c_str());
            return 1;
        case MOTOR_DRIVE:
            fprintf(stdout, "COMMAND IS MOTOR_DRIVE!\n");
            //robot.motor_drive();
            return 1;
        case MOTOR_STOP:
            fprintf(stdout, "COMMAND IS MOTOR_STOP!\n");
            //robot.motor_stop();
            return 1;
        // do things for switch
        case DRILL_RUN:
            // runs drill
            fprintf(stdout, "COMMAND IS DRILL_RUN!\n");
            return 1;
        case DRILL_STOP:
            // stops drill
            fprintf(stdout, "COMMAND IS DRILL_STOP!\n");
            return 1;
        case DRILL_CYCLE:
            // runs drill at duty cycle
            fprintf(stdout, "COMMAND IS DRILL_CYCLE!\n");
            return 1;
        case NONE:
        default:
            //string text = tk->text;
            msg = "Error: Unknown command!\n";// + text;
            sendToClient(*client_fd, msg.c_str());
            // not a built-in command
            return 0;
    }
}

static void listCommands(int client_fd) {
    std::string msg;

    msg = "Help - Commands:\n";
    sendToClient(client_fd, msg.c_str());
    /*
    msg = "help - prints this message\n";
    sendToClient(client_fd, msg.c_str());
    msg = "start - initializes system\n";
    sendToClient(client_fd, msg.c_str());
    msg = "standby - stops all components, system in standby\n";
    sendToClient(client_fd, msg.c_str());
    msg = "temp - returns current temperature reading\n";
    sendToClient(client_fd, msg.c_str());
    msg = "dtemp - returns temperature difference from startup\n";
    sendToClient(client_fd, msg.c_str());
    msg = "curr - returns current current reading\n";
    sendToClient(client_fd, msg.c_str());
    msg = "level - returns current level reading\n";
    sendToClient(client_fd, msg.c_str());
    msg = "wob - returns current force reading\n";
    sendToClient(client_fd, msg.c_str());
    msg = "data - returns the data file\n";
    sendToClient(client_fd, msg.c_str());
    msg = "motor_drive - drive the motor\n";
    sendToClient(client_fd, msg.c_str());
    msg = "motor_stop - stops the motor\n";
    sendToClient(client_fd, msg.c_str());
    msg = "drill_run - runs the drill\n";
    sendToClient(client_fd, msg.c_str());
    msg = "drill_stop - stops the drill\n";
    sendToClient(client_fd, msg.c_str());
    msg = "drill_cycle - changes drill pwm\n";
    sendToClient(client_fd, msg.c_str());
    msg = "auto - puts system in auto mode\n";
    sendToClient(client_fd, msg.c_str());
    msg = "quit - shutdown the entire system, including server\n";
    sendToClient(client_fd, msg.c_str());
    */

    return;
}

/*
    SIGNAL HANDLERS
*/
static void sigint_handler(int sig) {
    //@TODO: need to fix this
    int old_errno = errno;

    sigset_t mask, prev;
    sigprocmask(SIG_BLOCK, &mask, &prev);

    shutdownSIG = 1;

    sigprocmask(SIG_SETMASK, &prev, NULL);
    errno = old_errno;

    exit(0);
    return;
}

static void sigpipe_handler(int sig) {
    int old_errno = errno;

    sigset_t mask, prev;
    sigprocmask(SIG_BLOCK, &mask, &prev);

    disconnected = 1;

    sigprocmask(SIG_SETMASK, &prev, NULL);
    errno = old_errno;
    return;
}


/*
    NEWTORK-SAFE FILE READ/WRITE (BUFFERED)
    from Computer Systems by Bryant & O'Hallaron
*/
ssize_t rio_readn(int fd, void *usrbuf, size_t n) {
    size_t nleft = n;
    ssize_t nread;
    char *bufp = (char *)usrbuf;

    while (nleft > 0) {
        if ((nread = read(fd, bufp, nleft)) < 0) {
            if (errno != EINTR) {
                return -1;  /* errno set by read() */
            }

            /* Interrupted by sig handler return, call read() again */
            nread = 0;
        } else if (nread == 0) {
            break;                  /* EOF */
        }
        nleft -= nread;
        bufp += nread;
    }
    return n - nleft;             /* Return >= 0 */
}

ssize_t rio_writen(int fd, void *usrbuf, size_t n) {
    size_t nleft = n;
    ssize_t nwritten;
    char *bufp = (char *)usrbuf;

    while (nleft > 0) {
        if ((nwritten = write(fd, bufp, nleft)) <= 0) {
            if (errno != EINTR) {
                return -1;       /* errno set by write() */
            }

            /* Interrupted by sig handler return, call write() again */
            nwritten = 0;
        }
        nleft -= nwritten;
        bufp += nwritten;
    }
    return n;
}

static ssize_t rio_read(rio_t *rp, char *usrbuf, size_t n) {
    int cnt;

    while (rp->rio_cnt <= 0) {      /* Refill if buf is empty */
        rp->rio_cnt = read(rp->rio_fd, rp->rio_buf, sizeof(rp->rio_buf));
        fprintf(stdout, "\t\t\t%s\n", strerror(errno));
        if (rp->rio_cnt < 0) {
            if (errno != EINTR) {
                return -1;          /* errno set by read() */
            }

            /* Interrupted by sig handler return, nothing to do */
        } else if (rp->rio_cnt == 0) {
            return 0;               /* EOF */
        } else {
            rp->rio_bufptr = rp->rio_buf;   /* Reset buffer ptr */
        }
    }

    /* Copy min(n, rp->rio_cnt) bytes from internal buf to user buf */
    cnt = n;
    if ((size_t) rp->rio_cnt < n) {
        cnt = rp->rio_cnt;
    }
    memcpy(usrbuf, rp->rio_bufptr, cnt);
    rp->rio_bufptr += cnt;
    rp->rio_cnt -= cnt;
    return cnt;
}

void rio_readinitb(rio_t *rp, int fd) {
    rp->rio_fd = fd;
    rp->rio_cnt = 0;
    rp->rio_bufptr = rp->rio_buf;
}

ssize_t rio_readnb(rio_t *rp, void *usrbuf, size_t n) {
    size_t nleft = n;
    ssize_t nread;
    char *bufp = (char *)usrbuf;

    while (nleft > 0) {
        if ((nread = rio_read(rp, bufp, nleft)) < 0) {
            return -1;          /* errno set by read() */
        } else if (nread == 0) {
            break;              /* EOF */
        }
        nleft -= nread;
        bufp += nread;
    }
    return (n - nleft);         /* return >= 0 */
}

ssize_t rio_readlineb(rio_t *rp, void *usrbuf, size_t maxlen) {
    size_t n;
    int rc;
    char c, *bufp = (char *)usrbuf;

    for (n = 1; n < maxlen; n++) {
        if ((rc = rio_read(rp, &c, 1)) == 1) {
            *bufp++ = c;
            if (c == '\n') {
                n++;
                break;
            }
        } else if (rc == 0) {
            if (n == 1) {
                return 0; /* EOF, no data read */
            } else {
                break;    /* EOF, some data was read */
            }
        } else {
            return -1;    /* Error */
        }
    }
    *bufp = 0;
    return n-1;
}
