#ifndef _COMPONENTS_H_
#define _COMPONENTS_H_

#include "libraries/pybind11/include/pybind11/embed.h"


class Therm {
    private:
        int bus_addr;
        float max_T;
        float iTemp;
    public:
        Therm(int bus_addr, float max_T);
        ~Therm();
        float read_temp();
        float D_temp();
};

class Amm {
    private:
        int bus_addr;
        float max_I;
    public:
        Amm(int bus_addr, float max_I);
        ~Amm();
        float read_curr();
};

class WLevel {
    private:
        int bus_start;
        int bus_end;
    public:
        WLevel(int bus_start, int bus_end/*, int sample_freq*/);
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

class Motor {
    private:
        pybind11::object L298N;
    public:
        Motor(int pinA, int en);
        ~Motor();
        void motor_drive(bool dir, int speed, int time);
        void motor_stop();
};

class Encoder {
    private:
        int fd;
        int ppr; // pulse per revolution
    public:
        Encoder();
        void reset();
        void getTicks();
        float calcVel(int n, int t);
        ~Encoder();
};

#endif
