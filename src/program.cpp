#include <thread>
#include <signal.h>
#include <errno.h>

#include "server/APESServer.h"
#include "shell/APESShell.h"
#include "misc/flags.h"
#include "misc/flags_set.h"

static void sigpipe_handler(int sig);
static void sigint_handler(int sig);

static void serverThread(int *shell_cfd);
static void shellThread(int *shell_cfd);

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

    int shell_cfd = -1;

    std::thread tServer(serverThread, &shell_cfd);
    std::thread tShell(shellThread, &shell_cfd);

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
static void serverThread(int *shell_cfd) {
    assert(shell_cfd != NULL);
    
    APESServer server = Server();
    while (server.sfd < 0) {
        server.serverSetup(port);
    }
    assert(server.sfd >= 0);
    server.run(shell_cfd);
    
    // control should never reach here
    return;
}

   
static void shellThread(int *shell_cfd) {
    assert(shell_cfd != NULL);

    APESShell shell = Shell();
    shell.run(shell_cfd);

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

    setDisconnected();
    //robot.standby();

    sigprocmask(SIG_SETMASK, &prev, NULL);
    errno = old_errno;

    return;
}

static void sigint_handler(int sig) {
    int old_errno = errno;

    sigset_t mask, prev;
    sigprocmask(SIG_BLOCK, &mask, &prev);

    setShutdownSIG();
    // robot.shutdown();

    sigprocmask(SIG_SETMASK, &prev, NULL);
    errno = old_errno;

    return;
}
