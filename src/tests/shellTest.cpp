#include "../shell/APESShell.h"
#include <cstdlib>
#include <unistd.h>
#include <string>

int main() {

    std::string cmd = "cmd.txt";
    std::string log = "log.txt";

    APESShell s = APESShell(cmd, log);
    s.run();

    return -1;
}
