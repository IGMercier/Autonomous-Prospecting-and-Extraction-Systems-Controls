#include "helper.h"
#include <cstring>

// modified from tshlab
int parseline(const char *cmdline, token *tk) {
    const char delims[] = " \t\r\n";
    char *buf;
    char *next;
    char *endbuf;

    if (cmdline == NULL) {
        return -1;
    }

    strncpy(tk->text, cmdline, MAXLINE);

    buf = tk->text;
    endbuf = tk->text + strlen(tk->text);

    tk->argc = 0;

    while (buf < endbuf) {
        buf += strspn(buf, delims); // skips by num of delimiters
        if (buf >= endbuf) { break; }

        next = buf + strcspn(buf, delims);

        *next = '\0'; // make buf into valid c string

        tk->argv[tk->argc] = buf;
        tk->argc = tk->argc + 1;

        if (tk->argc >= MAXARGS - 1) { break; }

        buf = next + 1;
    }

    tk->argv[tk->argc] = NULL;
    if (tk->argc == 0) { return -1; }

    if ((strcmp(tk->argv[0], "quit")) == 0) {
        tk->command = QUIT;
    } else if ((strcmp(tk->argv[0], "start")) == 0) {
        tk->command = START;
    } else if ((strcmp(tk->argv[0], "standby")) == 0) {
        tk->command = STANDBY;
    } else if ((strcmp(tk->argv[0], "temp")) == 0) {
        tk->command = TEMP;
    } else if ((strcmp(tk->argv[0], "dtemp")) == 0) {
        tk->command = DTEMP;
    } else if ((strcmp(tk->argv[0], "curr")) == 0) {
        tk->command = CURR;
    } else if ((strcmp(tk->argv[0], "level")) == 0) {
        tk->command = WLEVEL;
    } else if ((strcmp(tk->argv[0], "wob")) == 0) {
        tk->command = WOB;
    } else if ((strcmp(tk->argv[0], "motor_drive")) == 0) {
        tk->command = MOTOR_DRIVE;
    } else if ((strcmp(tk->argv[0], "motor_stop")) == 0) {
        tk->command = MOTOR_STOP;
    } else if ((strcmp(tk->argv[0], "drill_run")) == 0) {
        tk->command = DRILL_RUN;
    } else if ((strcmp(tk->argv[0], "drill_stop")) == 0) {
        tk->command = DRILL_STOP;
    } else if ((strcmp(tk->argv[0], "drill_cycle")) == 0) {
        tk->command = DRILL_CYCLE;
    } else if ((strcmp(tk->argv[0], "auto")) == 0) {
        if ((strcmp(tk->argv[1], "on")) == 0) {
            tk->command = AUTO_ON;
        } else if ((strcmp(tk->argv[1], "off")) == 0) {
            tk->command = AUTO_OFF;
        } else {
            tk->command = NONE;
        }
    } else if ((strcmp(tk->argv[0], "help")) == 0) {
        tk->command = HELP;
    } else {
        tk->command = NONE;
    }

    return 0;
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
