#ifndef _FLAGS_H_
#define _FLAGS_H_

#include <mutex>
#include <deque>

#define VERBOSE 1

typedef struct sysArgs {
    std::mutex *cmd_mtx;
    std::mutex *log_mtx;
    std::deque<char *> *cmdq;
    std::deque<char *> *logq; 
} sysArgs;
#endif
