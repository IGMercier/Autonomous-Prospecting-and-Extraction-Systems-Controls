
#ifndef _HELPER_H_
#define _HELPER_H_

/*
    COMMAND & PARSING DEFINITIONS AND HELPER FUNCTIONS
*/
#define MAXARGS  128
#define MAXLINE  8192

typedef enum command_state {
    START,
    HELP,
    DATA,
    NONE,
    STANDBY,
    TEMP,
    DTEMP,
    CURR,
    WLEVEL,
    WOB,
    MOTOR_DRIVE,
    MOTOR_STOP,
    DRILL_RUN,
    DRILL_STOP,
    DRILL_CYCLE,
    AUTO_ON,
    AUTO_OFF,
    QUIT
} command_state;

typedef struct token {
    char text[MAXLINE]; 
    int argc;
    char *argv[MAXARGS];
    command_state command;
} token;

int parseline(const char *cmdline, token *tk);


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
