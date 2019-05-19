#include "commands.h"
//#include "components.h"
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
//#include "APES.h"
//#include <string.h>
//#include <cstring>

/*
    Setup Server
    Wait for input
    Evaluate input and add to job list, wait for more input
    separately execute each job in job list
*/

using std::string;

static int server_fd;
static volatile int disconnected = 1;
//APES robot;

int serverSetup(int port);
void  clientSetup();
static int readFromClient(int client_fd, char *cmdline);
static int sendToClient(int client_fd, const char *msg);
static int evaluate(int *client_fd, const char *cmdline, token *tk);
static int command(int *client_fd, token *tk);
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
       [X] setup robot
       [X] put robot in standby
       [] when connected:
         [X] accept commands
	     [] execute commands
       [] when disconnected:
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
    fprintf(stdout, "Attempting serverSetup! ");
    while ((server_fd = serverSetup(port)) < 0) {
        fprintf(stderr, "ERROR: %s\n", strerror(errno));
        fprintf(stdout, "Retrying...\n");
        
        // retry to setup
    }
    assert(server_fd >= 0);

    fprintf(stdout, "Finished serverSetup!\n");

    fprintf(stdout, "Attempting clientSetup!\n");
    clientSetup();

    // control should never reach here
    shutdown(-1);
    return -1;
}

/*
    SERVER/CLIENT FUNCTIONS
*/
int serverSetup(int port) {
    int flags = 1;
    struct sockaddr_in serveraddr;
    int sfd;

    if ((sfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        return -1;
    }

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
    string msg = "Connected...!\n";
    sendToClient(client_fd, msg.c_str());
    fprintf(stdout, msg.c_str());

    while (!disconnected) {
        char *cmdline = (char *)calloc(MAXLINE, sizeof(char));
        token tk;

        readFromClient(client_fd, cmdline);
        evaluate(&client_fd, cmdline, &tk);

        free(cmdline);

    }

    //robot.standby();
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
    //robot.finish();

    string msg = "Shutting down!\n";
    sendToClient(client_fd, msg.c_str());
    fprintf(stdout, msg.c_str());

    if (close(client_fd) < 0) {
        fprintf(stderr, "ERROR: %s\n", strerror(errno));
    }
    if (close(server_fd) < 0) {
        fprintf(stderr, "ERROR: %s\n", strerror(errno));
    }
    exit(0);
}

static int evaluate(int *client_fd, const char *cmdline, token *tk) {
    assert(cmdline != NULL);
    assert(tk != NULL);

    parseline(cmdline, tk);

    return command(client_fd, tk);
}

static int command(int *client_fd, token *tk) {
    assert(tk != NULL);

    command_state command = tk->command;
    string msg;

    switch (command) {
        case START:
            // setup robot and retry on fail
            //while (robot.setup() < 0) {
            //    fprintf(stderr, "ERROR: APES system setup failure!\n");
            //    fprintf(stdout, "Retrying...\n");
            //    robot.finish();
            //}

            // put in standby
            //robot.standby();
            msg = "ROBOT STARTED\n";
            sendToClient(*client_fd, msg.c_str());
            //fprintf(stdout, "COMMAND IS START!\n");
            return 1;
        case HELP:
            //string helpLines[] = listCommands();
            //int size = *(&helpLines + 1) - helpLines;
            //for (int i = 0; i < size; i++) {
            //    sendToClient(*client_fd, helpLines[i].c_str());
            //}
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
            fprintf(stdout, "COMMAND IS AUTO!\n");
            return 1;
        case TEMP:
            /*
            float temp;
            temp = robot.read_temp();
            printf(stdout, "Temp (@time): %f\n", temp);
            */
            msg = "TEMP MODE\n";
            fprintf(stdout, "COMMAND IS TEMP!\n");
            sendToClient(*client_fd, msg.c_str());
            return 1;
        case DTEMP:
            /*
            float dtemp;
            dtemp = robot.D_temp();
            printf(stdout, "Temp since init: %f\n", dtemp);
            */
            msg = "DTEMP MODE\n";
            fprintf(stdout, "COMMAND IS DTEMP!\n");
            sendToClient(*client_fd, msg.c_str());
            return 1;
        case CURR:
            /*
            float curr;
            curr = robot.read_curr();
            printf(stdout, "Curr (@time): %f\n", curr);
            */
            msg = "CURR MODE\n";
            fprintf(stdout, "COMMAND IS CURR!\n");
            sendToClient(*client_fd, msg.c_str());
            return 1;
        case LEVEL:
            /*
            int level;
            level = robot.read_level();
            printf(stdout, "Level (@time): %d\n", level);
            */
            msg = "LEVEL MODE\n";
            sendToClient(*client_fd, msg.c_str());
            fprintf(stdout, "COMMAND IS LEVEL!\n");
            return 1;
        case STANDBY:
            //robot.standby();
            msg = "ROBOT IN STANDBY\n";
            sendToClient(*client_fd, msg.c_str());
            return 1;
        case WOB:
            /*
            float force;
            force = robot.read_wob();
            printf(stdout, "Force (@time): %f\n", force);
            */
            fprintf(stdout, "COMMAND IS WOB!\n");
            return 1;
        case DATA:
            //robot.read_data();
            fprintf(stdout, "COMMAND IS DATA!\n");
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

/*
    SIGNAL HANDLERS
*/
static void sigint_handler(int sig) {
    //@TODO: implement
    exit(0);
}

static void sigpipe_handler(int sig) {
    fprintf(stdout, "CLIENT HAS DISCONNECTED INVOLUNTARILY!\n");
    disconnected = 1;
    return;
}


/*
    NEWTORK-SAFE FILE READ/WRITE
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
