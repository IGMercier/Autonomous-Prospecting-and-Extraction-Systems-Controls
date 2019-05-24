#ifndef _FLAGS_H_
#define _FLAGS_H_

#include <signal.h>
#include <mutex>

#define VERBOSE 1
extern sig_atomic_t disconnected;
extern sig_atomic_t shutdownSIG;
extern std::mutex fd_mtx;


#endif
