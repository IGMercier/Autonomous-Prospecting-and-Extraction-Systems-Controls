#ifndef __COMPONENTS_H__
#define __COMPONENTS_H__

#include "libraies/pybind11/include/pybind11/embed.h"
#include <ctime>

typedef struct dataPt {
    union data {
        int dataI;
        float dataF;
    };
    time_t time;
} dataPt;

class Therm {
    private:
        int bus_addr;
        float max_T;
        float iTemp;
    public:
        Therm(int bus_addr, float max_T);
        ~Therm();
        float read_temp(therm_t thermo);
        float D_temp(therm_t thermo);
};

class Amm {
    private:
        int bus_addr;
        float max_I;
    public:
        Amm(int bus_addr, float max_I);
        ~Amm();
        float read_curr(amm_t ammeter);
};

class Level {
    private:
        int bus_start;
        int bus_end;
    public:
        Level(int bus_start, int bus_end, int sample_freq);
        ~Level();
        int read_level(level_t wlevel);
};

class Wob {
    private:
        pybind11::object HX711;
    public:
        Wob();
        ~Wob();
        float read_wob(wob_t loadcell);
};

class Motor {
    private:
        pybind11::object L298N;
    public:
        Motor(int pinA, int pinB);
        ~Motor();
        void motor_drive(bool dir, int speed, int time);
        void motor_stop();
};

#endif
