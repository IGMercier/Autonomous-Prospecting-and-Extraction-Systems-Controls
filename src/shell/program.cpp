#include "shellBase.h"
#include <cstdlib>
#include <unistd.h>

int main() {
    int file = STDIN_FILENO;
    ShellBase s = ShellBase(&file);
    s.run();

    return -1;
}
