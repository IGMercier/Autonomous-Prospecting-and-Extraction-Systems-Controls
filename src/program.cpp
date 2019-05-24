#include <thread>
#include <signal.h>
#include <errno.h>

#include "server/APESServer.h"
#include "shell/APESShell.h"
#include "misc/flags.h"

static void sigpipe_handler(int sig);
static void sigint_handler(int sig);

static void serverThread(int *cfd);
static void shellThread(int *cfd);

int main(int argc, char** argv) {
    signal(SIGPIPE, sigpipe_handler);
    signal(SIGINT, sigint_handler);
    signal(SIGTSTP, SIG_IGN);

    int port;
    if (argc < 2) {
        port = 16778;
    } else {
        port = atoi(argv[1]);
    }

    std::thread tServer(serverThread);
    std::thread tShell(shellThread);

    if (tServer.joinable()) {
        tServer.join();
    }

    if (tShell.joinable()) {
        tShell.join();
    }

    // control should never reach here
    return -1;

}


/*
    THREAD CALLBACKS
*/
static void serverThread() {
    assert(shell_cfd != NULL);
    
    APESServer server = APESServer();
    while (server.sfd < 0) {
        server.serverSetup(port);
    }
    assert(server.sfd >= 0);
    server.run();
    
    // control should never reach here
    return;
}

   
static void shellThread() {
    assert(shell_cfd != NULL);

    APESShell shell = APESShell();
    shell.run();

    // control should never reach here
    return;
}


/*
    SIGNAL HANDLERS
*/
static void sigpipe_handler(int sig) {
    int old_errno = errno;

    sigset_t mask, prev;
    sigprocmask(SIG_BLOCK, &mask, &prev);

    disconnected = 1;
    //robot.standby();

    sigprocmask(SIG_SETMASK, &prev, NULL);
    errno = old_errno;

    return;
}

static void sigint_handler(int sig) {
    int old_errno = errno;

    sigset_t mask, prev;
    sigprocmask(SIG_BLOCK, &mask, &prev);

    shutdownSIG = 1;
    // robot.shutdown();

    sigprocmask(SIG_SETMASK, &prev, NULL);
    errno = old_errno;

    return;
}
