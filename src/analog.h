#ifndef ANALOG_H
#define ANALOG_H

typedef struct therm {
    int bus_addr;
    float max_T;
    float iTemp;
} therm;

typedef therm* therm_t;

therm_t make_therm(int bus_addr, float max_T);
float read_temp(therm_t thermo);
float D_temp(therm_t thermo);
void freeTherm(therm_t thermo);


typedef struct amm {
    int bus_addr;
    float max_I;
} amm;

typedef amm* amm_t;

amm_t make_amm(int bus_addr, float max_I);
float read_curr(amm_t ammeter);
void freeAmm(amm_t ammeter);


typedef struct level {
    int bus_start;
    int bus_end;
} level;

typedef level* level_t;

level_t make_level(int bus_start, int bus_end, int sample_freq);
int read_level(level_t wlevel);
void freeLevel(level_t wlevel);

#endif
