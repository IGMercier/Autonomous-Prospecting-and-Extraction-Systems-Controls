#include "../shell/APESShell.h"
#include <cstdlib>
#include <unistd.h>
#include <string>

int main() {

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
    APESShell s = APESShell(cmdq, logq);
    s.run();

    return -1;
}
