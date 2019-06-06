#include "../APESsys/components.h"

#include <csignal>

static volatile int off;

static void sig_handler(int sig);

int main(int argc, char **argv) {
    wiringPiSetupPhys();
    signal(SIGINT, sig_handler);

    off = 0;

    /*
    Drill* drill = new Drill(11, 7);
    drill->drill_run(50, 100000);

    sleep(1);

    drill->drill_stop();
    delete drill;
    */

   /* 
    Stepper* stepper = new Stepper(35, 36);
    stepper->stepper_drive(1);

    sleep(10);
    
    stepper->stepper_stop();

    delete stepper;
    */
    
    /*
    int fd = wiringPiI2CSetup(0x20);
    Encoder* encoder = new Encoder(fd, 1024);
    while (1) {
        unsigned int pulse = encoder->getPulse();
        printf("%u\n", pulse);
        sleep(2);
    }
    */

    /*
    Pump *pump = new Pump(15, 12);
    ump->pump_drive(1, 50);

    sleep(2);
    pump->pump_stop();

    delete pump;
    */
/*
    py::initialize_interpreter();

    Wob *wob = new Wob(22, 12);
    while (!off) {
        float force = wob->read_wob();
        printf("Force reading: %f\n", force);
        sleep(2);
    }

    delete wob;
    
    py::finalize_interpreter();
*/
    int fd = wiringPiI2CSetup(0x26);
    Encoder *enc = new Encoder(fd, 1024)
    unsigned int prev = enc->getPulse();

    Stepper *motor = new Stepper(35, 36);
    motor->stepper_drive(250, 6000);
    
    unsigned int curr = enc->getPulse();
    
    printf("%u, %u, %u", prev, curr, curr-prev);
    motor->stepper_drive(-250, 6000);
    /*
    Relay *relay = new Relay(16);
    relay->turnOn();
    
    sleep(2);
    relay->turnOff();

    delete relay;
    */
}

static void sig_handler(int sig) {
    off = 1;
}
