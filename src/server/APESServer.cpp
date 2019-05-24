#include "APESServer.h"
#include <cstdlib>
#include <cstdio>
#include <csignal>
#include <unistd.h>
#include <cstring>
#include <assert.h>
#include <errno.h>
#include <thread>
#include <mutex>
//#include "../misc/flags.h"

#define MAXLINE 1024

static void sigpipe_handler(int sig);
std::mutex cmd_mtx;
std::mutex log_mtx;
volatile sig_atomic_t disconnected = 1;
volatile sig_atomic_t shutdownSIG = 0;

static void connection(APESServer *server);

APESServer::APESServer(char *cmdfile, char *logfile) {
    signal(SIGPIPE, sigpipe_handler);
    //signal(SIGINT, sigint_handler);
    if (cmdfile == NULL) {
        this->cmdfile = ".cmdline.txt";
    } else {
        this->cmdfile = cmdfile;
    }
    if (logfile == NULL) {
        this->logfile = ".log.txt";
    } else {
        this->logfile = logfile;
    }
}

void APESServer::run() {
    assert(this->sfd >= 0);

    while (!shutdownSIG) {
        int val = createClient();
        if (val == -1) {
            this->cfd = -1;
            continue;
        } else if (val == -2) {
            close(this->cfd);
            this->cfd = -1;
            continue;
        }

        std::thread child(connection, this);
        child.detach();
    }
    shutdown();
    return; // kills server thread in main program
}

static void connection(APESServer *server) {
    assert(server != NULL);
    
    disconnected = 0;

    std::string msg = "Connected!\n";
    server->sendToClient(msg);
    fprintf(stdout, msg.c_str());

    while (!disconnected && !shutdownSIG) {
        server->setClientSockOpts();
        //fprintf(stdout, "in loop\n");
        //char line[MAXLINE];
        //server->readFromClient(line, MAXLINE);

        //fprintf(stdout, "GREAT\n");
        //std::unique_lock<std::mutex> clock(cmd_mtx);
        //FILE *cmd = fopen(server->cmdfile, "w");
        //fprintf(cmd, line);
        //fclose(cmd);
        //clock.unlock();


        //std::unique_lock<std::mutex> llock(log_mtx);
        //fprintf(stdout, "I'D BE READING FROM LOGFILE NOW!\n");
        //FILE *log = fopen(server->logfile, "r");
        //char *rc = NULL;
        //size_t len = 0;
        //getline(&rc, &len, log);
        //llock.unlock();

        //server->sendToClient(rc);
    }

    close(server->cfd);
    server->cfd = -1;

    fprintf(stdout, "Disconnected!\n");
    return;

}

void APESServer::shutdown() {
    std::string msg = "Server shutting down!\n";
    sendToClient(msg);
    fprintf(stdout, "%s", msg.c_str());

    if (this->cfd >= 0) {
        if (close(this->cfd) < 0) {
            fprintf(stderr, "ERROR: %s\n", strerror(errno));
        }
    }
    if (this->sfd >= 0) {
        if (close(this->sfd) < 0) {
            fprintf(stderr, "ERROR: %s\n", strerror(errno));
        }
    }
    return;
}

APESServer::~APESServer() {
}

/* REMOVE AFTER TESTING */
static void sigpipe_handler(int sig) {
    int old_errno = errno;
    sigset_t mask, prev;
    sigprocmask(SIG_BLOCK, &mask, &prev);

    fprintf(stdout, "IN SIG HANDLER\n");

    disconnected = 1;

    sigprocmask(SIG_SETMASK, &prev, NULL);
    errno = old_errno;
    return;
}

