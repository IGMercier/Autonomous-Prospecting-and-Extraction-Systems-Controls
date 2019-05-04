#include <stdio.h>
#include <stdlib.h>
#include "analog.h"

/*
    HELPER FUNCTIONS
*/
int readADC(int bus, int channel) {
    if ((bus < 0) || (bus > 1)) {
        return -1;
    }
    if (channel > 7) {
        return -1;
    }

    unsigned char data[3];
    data[0] = 0b1;
    data[1] = (0b1000 + channel) << 4;
    data[2] = 0b0;
    wiringPiSPIDataRW(bus, data, 3);

    int datum = ((data[1] & 0b11) << 8) + data[2];

    return datum;
}


/*
    THERMOMETER FUNCTIONS
*/
therm_t make_therm(int bus_addr, float max_T) {
    if ((bus_addr > 7) || (bus_addr < 0)) {
        fprintf(stderr, "ERROR: thermometer bus address takes values 0-7!\n");
        fprintf(stderr, "ERROR: thermometer instatiation failed!\n");
        return;
    }

    therm_t thermo = calloc(1, sizeof(therm));

    thermo->bus_addr = bus_addr;
    thermo->max_T = max_T;

    thermo->iTemp = read_temp(thermo);
    
    return thermo;
}

float read_temp(therm_t thermo) {
    int bus, channel;

    // @TODO: how is bus_addr specified???

    return (float)readADC(bus, channel);
}

float D_temp(therm_t thermo) {
    float fTemp = read_temp(thermo->bus_addr);
    return fTemp - thermo->iTemp;
}


/*
    AMMETER FUNCTIONS
*/
amm_t make_amm(int bus_addr, float max_I) {
    if ((bus_addr > 7) || (bus_addr < 0)) {
        fprintf(stderr, "ERROR: ammeter bus address takes values 0-7!\n");
        fprintf(stderr, "ERROR: ammeter instatiation failed!\n");
        return;
    }

    amm_t ammeter = calloc(1, sizeof(amm));
    
    ammeter->bus_addr = bus_addr;
    ammeter->max_I = max_I;

    return ammeter;
}

float read_curr(amm_t ammeter) {
    int bus, channel;

    // @TODO: how is bus_addr specified???

    return (float)readADC(bus, channel);
}


/*
    WATER LEVEL FUNCTIONS
*/
level_t make_level(int bus_start, int bus_end) {
    if ((bus_start > 7) || (bus_start < 0)) {
        fprintf(stderr, "ERROR: level bus_start address takes values 0-7!\n");
        fprintf(stderr, "ERROR: level instatiation failed!\n");
        return;
    }
    if ((bus_end > 7) || (bus_end < 0)) {
        fprintf(stderr, "ERROR: level bus_end address takes values 0-7!\n");
        fprintf(stderr, "ERROR: level instatiation failed!\n");
        return;
    }

    // requires that bus_start <= bus_end
    if (bus_start > bus_end) {
        fprintf(stderr, "ERROR: level requires that bus_start address");
        fprintf(stderr, " <= bus_end address\n");
        fprintf(stderr, "ERROR: level instatiation failed!\n");
        return;
    }


    level_t wlevel = calloc(1, sizeof(level));

    wlevel->bus_start = bus_start;
    wlevel->bus_end = bus_end;

    return wlevel;
}

int read_level(level_t wlevel) {
    /*@TODO:
      reads from each ADC channel
      assumes 1 == water at that level
      assumes channel 0 is the lowest level
        and channel 7 is the highest level
    */
    int bus;
    int level = 0;
    int channel = wlevel->bus_start;
    for ( ; channel < wlevel->bus_end; channel++) {
        if (readADC(bus, channel) == 1) {
            level = channel;
        }
    }

    return level;
}
