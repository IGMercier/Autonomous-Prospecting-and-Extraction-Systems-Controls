
#include "components.h"

int main() {
    Amm *a = new Amm(0, 6);
    while (1)
        printf("Current Reading: %f\n", a->read_curr());
}
