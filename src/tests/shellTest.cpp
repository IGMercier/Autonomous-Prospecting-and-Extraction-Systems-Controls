#include "../shell/APESShell.h"
#include <cstdlib>
#include <unistd.h>

int main() {

    std::string c = "cmd.txt";
    std::string l = "log.txt";
    APESShell s = APESShell(c, l);
    s.run();

    return -1;
}
