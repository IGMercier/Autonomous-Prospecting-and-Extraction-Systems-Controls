#ifndef ANALOG_H
#define ANALOG_H

typedef struct therm {
    int bus_addr;
    float max_T;
    float iTemp;
} therm;

typedef therm* therm_t;

therm_t make_therm(int bus_addr, float max_T);
float read_temp();
float D_temp();


typedef struct amm {
    int bus_addr;
    float max_I;
} amm;

typedef amm* amm_t;

amm_t make_amm(int bus_addr, float max_I);
float read_curr();


typedef struct level {
    int bus_start;
    int bus_end;
} level;

typedef level* level_t;

level_t make_level(int bus_start, int bus_end, int sample_freq);
int read_level();

#endif
