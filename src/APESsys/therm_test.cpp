#include <cstdio>
#include <wiringPiSPI.h>
#include "components.h"

int main () {
    wiringPiSPISetup(0, 500000);    

    Therm *t = new Therm(7);
    while (1) {
       int raw = t->read_temp();
       printf("V: %i\n", raw);
       //float temp = (raw - 20) * 0.424 - 40;
       //printf("T: %f\n", temp);
    }
    return 0;
}
