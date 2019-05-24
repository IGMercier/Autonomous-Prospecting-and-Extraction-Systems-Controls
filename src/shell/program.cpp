#include "APESShell.h"
#include <cstdlib>
#include <unistd.h>

int main() {
    int file = STDIN_FILENO;
    APESShell s = APESShell(&file);
    s.run();

    return -1;
}
