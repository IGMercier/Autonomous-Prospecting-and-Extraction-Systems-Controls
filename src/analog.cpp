#include <stdio.h>
#include "analog.h"


/*
    THERMOMETER FUNCTIONS
*/
Therm::Therm(int bus_addr, float max_T, int sample_freq) {
    if ((bus_addr > 7) || (bus_addr < 0)) {
        fprintf(stderr, "ERROR: thermometer bus address takes values 0-7!\n");
        fprintf(stderr, "ERROR: thermometer instatiation failed!\n");
        return;
    }
    if (sample_freq < 0) {
        fprintf(stderr, "ERROR: thermometer sample_freq takes values >= 0!\n");
        fprintf(stderr, "ERROR: thermometer instatiation failed!\n");
        return;
    }

    this->bus_addr = bus_addr;
    this->max_T = max_T;
    this->sample_freq = sample_freq;

    this->iTemp = this->read_temp();
    
    return;
}

float Therm::read_temp() {

}

float Therm::D_temp() {
    float fTemp = this->read_temp();
    return fTemp - this->iTemp;
}


/*
    AMMETER FUNCTIONS
*/
Amm::Amm(int bus_addr, float max_I, int sample_freq) {
    if ((bus_addr > 7) || (bus_addr < 0)) {
        fprintf(stderr, "ERROR: ammeter bus address takes values 0-7!\n");
        fprintf(stderr, "ERROR: ammeter instatiation failed!\n");
        return;
    }
    if (sample_freq < 0) {
        fprintf(stderr, "ERROR: ammeter sample_freq takes values >= 0!\n");
        fprintf(stderr, "ERROR: ammeter instatiation failed!\n");
        return;
    }

    this->bus_addr = bus_addr;
    this->max_I = max_I;
    this->sample_freq = sample_freq;

    return;
}

float Amm::read_curr() {

}


/*
    WATER LEVEL FUNCTIONS
*/
Level::Level(int bus_start, int bus_end, int sample_freq) {
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
    if (sample_freq < 0) {
        fprintf(stderr, "ERROR: level sample_freq takes values >= 0!\n");
        fprintf(stderr, "ERROR: level instatiation failed!\n");
        return;
    }

    this->bus_start = bus_start;
    this->bus_end = bus_end;
    this->sample_freq = sample_freq;

    return;
}

int Level::read_level() {

}
