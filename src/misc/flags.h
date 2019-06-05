#ifndef _FLAGS_H_
#define _FLAGS_H_

#include <mutex>
#include <deque>
#include <string>
#include <assert.h>

#define VERBOSE 1
#define ASSERT
#define MAXLINE 1024

#ifndef ASSERT
#define dbg_assert(...) assert(__VA_ARGS__)
#else
#define dbg_assert(...)
#endif

extern std::string shutdown_tag;
extern std::string data_tag;

typedef struct sysArgs {
    std::mutex *cmd_mtx;
    std::mutex *log_mtx;
    std::mutex *data_mtx;
    std::deque<std::string> *cmdq;
    std::deque<std::string> *logq;
    std::string datafile;
} sysArgs;
#endif
