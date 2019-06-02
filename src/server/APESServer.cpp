#include <cstdlib>
#include <cstdio>
#include <csignal>
#include <fstream>
#include <unistd.h>
#include <cstring>
#include <assert.h>
#include <errno.h>
#include <netinet/tcp.h>

#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <atomic>

#include "APESServer.h"

<<<<<<< HEAD
static std::string delim = "<END>";
=======
static std::string delim = "\r\n";
>>>>>>> 0a758d23dc5bbd559c7a7adea751933b24e3ceba
static std::atomic_int shutdown_sig = {0};

APESServer::APESServer(sysArgs *args) {
    signal(SIGPIPE, SIG_IGN);
    assert(args != NULL);
    assert(args->cmd_mtx != NULL);
    assert(args->log_mtx != NULL);
    assert(args->data_mtx != NULL);
    assert(args->cmdq != NULL);
    assert(args->logq != NULL);
    assert(!args->datafile.empty());

    this->cmd_mtx = args->cmd_mtx;
    this->log_mtx = args->log_mtx;
    this->data_mtx = args->data_mtx;

    this->cmdq = args->cmdq;
    this->logq = args->logq;

    this->datafile = datafile;
}

void APESServer::run(int port) {
    while (this->sfd < 0) {
        createServer(port);
    }
    assert(this->sfd >= 0);
        
   setClientSockOpts();
   setServerSockOpts();

    while (!shutdown_sig.load()) {
        createClient();

        assert(this->cfd >= 0);
        execute();
    }

    return;
}

void APESServer::execute() {
    char *cmdline = new char[MAXLINE];
    while (this->cfd > 0 && !shutdown_sig.load()) {

        int rc;
        memset(cmdline, 0, MAXLINE);
        rc = readFromClient(cmdline);
        if (rc < 0) {
            delete cmdline;
            disconnected();
            return;
        }
        std::string buf = cmdline;

        printf("%s\n", cmdline);
        
        // writes to command file for shell to read
        std::unique_lock<std::mutex> cmdlock(*(this->cmd_mtx));
        this->cmdq->push_back(buf);
        cmdlock.unlock();
    }
    
    delete cmdline;

    if (!shutdown_sig.load()) {
        disconnected();
    }

    return;

}

void APESServer::write() {
	
    while (1) {
        while (this->cfd >= 0) { 
            int rc;
        	
            std::unique_lock<std::mutex> loglock(*(this->log_mtx));
            unsigned int logs = this->logq->size();
            for (unsigned int i = 0; i < logs; i++) {
                std::string logline = this->logq->at(i);
    
                if (logline == shutdown_tag) {
                    shutdown_sig.store(1);
		    shutdown();
                    return;
                }
    
                if ((rc = sendToClient(logline) < 0)) {
                    disconnected();
                    break;
                }
            }
            this->logq->clear();
            loglock.unlock();
    
            /*
            // reads off data file
            std::unique_lock<std::mutex> datalock(*(this->data_mtx));
            {
                std::ifstream data(this->datafile);
                if (data) {
                    std::string dataline;
                    while (getline(data, dataline)) {
                        sendToClient(dataline.c_str());
                    }
                }
            }
            datalock.unlock();
            */
	    }
    }
}

void APESServer::disconnected() {
    // if disconnected, clears command queue and inserts standby command
    std::unique_lock<std::mutex> cmdlock(*(this->cmd_mtx));
    this->cmdq->clear();
    std::string buf = "standby";
    this->cmdq->push_back(buf);
    this->cmdq->push_back(shutdown_tag);
    cmdlock.unlock();
    
    close(this->cfd);
    this->cfd = -1;
    print("Disconnected!");

}

void APESServer::shutdown() {
    if (this->cfd >= 0) {
        if (close(this->cfd) < 0) {
            //print(strerror(errno));
        }
    }
    if (this->sfd >= 0) {
        if (close(this->sfd) < 0) {
            //print(strerror(errno));
        }
    }
    exit(0);
}

APESServer::~APESServer() {
}

