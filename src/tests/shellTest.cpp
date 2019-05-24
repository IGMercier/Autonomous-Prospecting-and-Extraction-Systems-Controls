#include "../shell/APESShell.h"
#include <cstdlib>
#include <unistd.h>

int main() {
    APESShell s = APESShell(NULL, NULL);
    s.run();

    return -1;
}
