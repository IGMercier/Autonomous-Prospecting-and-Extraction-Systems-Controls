#include "../APESsys/components.h"

int main(int argc, char **argv) {
    wiringPiSetupPhys();

    /*
    Drill* drill = new Drill(11, 7);
    drill->drill_run(50, 100000);

    sleep(1);

    drill->drill_stop();
    delete drill;
    */

    Stepper* stepper = new Stepper(36, 35);
    stepper->stepper_drive(1, 100, 1000);

    sleep(10);
    
    stepper->stepper_stop();

    delete stepper;


    //Motor* pump = new Motor();
    Relay *relay = new Relay(16);
    relay->turnOn();
    
    sleep(2);
    relay->turnOff();

    delete relay;
}
