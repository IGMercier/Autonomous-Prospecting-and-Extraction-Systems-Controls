#ifndef _FLAGS_H_
#define _FLAGS_H_

#include <mutex>
#include <deque>
#include <string>

#define VERBOSE 1
#define MAXLINE 1024

typedef struct sysArgs {
    std::mutex *cmd_mtx;
    std::mutex *log_mtx;
    std::deque<std::string> *cmdq;
    std::deque<std::string> *logq; 
} sysArgs;
#endif
