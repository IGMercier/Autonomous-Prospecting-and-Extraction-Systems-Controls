#ifndef _COMPONENTS_H_
#define _COMPONENTS_H_

#include "libraries/pybind11/include/pybind11/embed.h"
#include <wiringPi.h>
#include <wiringPiSPI.h>
#include <wiringPiI2C.h>

class Therm {
    private:
        int bus;
        int chan;
        float iTemp;
    public:
        Therm(int bus, int chan);
        ~Therm();
        float read_temp();
        float D_temp();
};

class Amm {
    private:
        int bus;
        int chan;
    public:
        Amm(int bus, int chan);
        ~Amm();
        float read_curr();
};

class WLevel {
    private:
        int bus;
        int chan_start;
        int chan_end;
    public:
        WLevel(int bus, int chan_start, int chan_end);
        ~WLevel();
        int read_wlevel();
};

class Wob {
    private:
        pybind11::object HX711;
    public:
        Wob(int pinA, int pinB);
        ~Wob();
        float read_wob();
};

class Encoder {
    private:
        int fd;
        int ppr; // pulse per revolution
    public:
        Encoder();
        void reset();
        unsigned int getPulse();
        float calcVel(int n, int t);
        ~Encoder();
};

class Drill {
    private:
        int en;
        int pwm;
    public:
        Drill(int en, int pwm);
        void drill_run(int dc);
        void drill_stop();
        void drill_cycle(int dc, int on_period, float freq);
        ~Drill();
};

class Motor {
    private:
        pybind11::object L298N;
    public:
        Motor(int pinA, int en);
        ~Motor();
        void motor_drive(bool dir, int speed, int time);
        void motor_stop();
};

class Solenoid {
    private:
        int pin;
    public:
        Solenoid(int pin);
        void openValve();
        void closeValve();
        ~Solenoid();
};

class DCHeater {
    private:
        int pin;
    public:
        DCHeater(int pin);
        void turnOn();
        void turnOff();
        ~DCHeater();
};

class Relay {
    private:
        int pin;
    public:
        Relay(int pin);
        void turnOn();
        void turnOff();
        ~Relay();
};

#endif
