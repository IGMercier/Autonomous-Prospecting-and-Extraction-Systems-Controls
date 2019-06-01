#include "../shell/APESShell.h"

int main() {


    std::mutex cmd_mtx;
    std::mutex log_mtx;

    std::deque<char *> *cmdq = new std::deque<char *>;
    std::deque<char *> *logq = new std::deque<char *>;

    cmdq->push_back("start");
    cmdq->push_back("standby");
    cmdq->push_back("data");
    cmdq->push_back("help");
    cmdq->push_back("quit");
    cmdq->push_back("auto on");
    cmdq->push_back("auto off");
    cmdq->push_back("temp");
    cmdq->push_back("dtemp");
    cmdq->push_back("curr");
    cmdq->push_back("wlevel");
    cmdq->push_back("wob");
    cmdq->push_back("motor drive");
    cmdq->push_back("motor stop");
    cmdq->push_back("drill run");
    cmdq->push_back("drill stop");
    cmdq->push_back("drill cycle joshua");
    cmdq->push_back("blarg");

    sysArgs args;
    args.cmd_mtx = &cmd_mtx;
    args.log_mtx = &log_mtx;
    args.cmdq = cmdq;
    args.logq = logq;

    APESShell s = APESShell(&args);
    s.run();

    return -1;
}
