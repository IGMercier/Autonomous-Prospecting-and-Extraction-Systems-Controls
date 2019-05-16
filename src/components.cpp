#include "analog.h"
#include <stdio.h>
#include <stdlib.h>

namespace py = pybind11;
using namespace py::literals;

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
Therm::Therm(int bus_addr, float max_T) {
    if ((bus_addr > 7) || (bus_addr < 0)) {
        fprintf(stderr, "ERROR: thermometer bus address takes values 0-7!\n");
        fprintf(stderr, "ERROR: thermometer instatiation failed!\n");
        return;
    }

    this.bus_addr = bus_addr;
    this.max_T = max_T;
    this.iTemp = read_temp(thermo);
}

float Therm::read_temp() {
    int bus, channel;

    // @TODO: how is bus_addr specified???

    return (float)readADC(bus, channel);
}

float Therm::D_temp(therm_t thermo) {
    float fTemp = read_temp(this.bus_addr);
    return fTemp - this.iTemp;
}


/*
    AMMETER FUNCTIONS
*/
Amm::Amm(int bus_addr, float max_I) {
    if ((bus_addr > 7) || (bus_addr < 0)) {
        fprintf(stderr, "ERROR: ammeter bus address takes values 0-7!\n");
        fprintf(stderr, "ERROR: ammeter instatiation failed!\n");
        return;
    }

    this.bus_addr = bus_addr;
    this.max_I = max_I;
}

float Amm::read_curr() {
    int bus, channel;

    // @TODO: how is bus_addr specified???

    return (float)readADC(bus, channel);
}


/*
    WATER LEVEL FUNCTIONS
*/
Level::Level(int bus_start, int bus_end) {
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


    this.bus_start = bus_start;
    this.bus_end = bus_end;
}

int Level::read_level() {
    /*@TODO:
      reads from each ADC channel
      assumes 1 == water at that level
      assumes channel 0 is the lowest level
        and channel 7 is the highest level
    */
    int bus;
    int level = 0;
    int channel = this.bus_start;
    for ( ; channel < this.bus_end; channel++) {
        if (readADC(bus, channel) == 1) {
            level = channel;
        }
    }

    return level;
}


/*
    WOB FUNCTIONS
*/
Wob::Wob() {
    // this assumes the pybind interpreter has been initialized
    // in  APES::setup()!
    py::object hx711 = py::module::import("libraries/hx711").attr("HX711");
    assert(hx711 != NULL);

    this.HX711 = hx711(5, 6);
    this.HX711.attr("set_reading_format")("byte_format" _a="MSB", "bit_format" _a="MSB");
    this.HX711.attr("set_reference_unit")(1);
    this.HX711.attr("reset")();
    this.HX711.attr("tare")();
}

float Wob::read_wob() {
    py::object value;
    value = this.HX711.attr("get_weight")("times" _a=5);
    this.HX711.attr("power_down")();
    this.HX711.attr("power_up")();

    //@TODO: look up how to convert pybind value to C++ primitive

    return 1;
}

Wob::~Wob() {
    // this assumes the pybind interpreter will be
    // finalized in APES::finish()
    if (this.HX711 != NULL) {
        this.HX711.attr("finish")();
        this.HX711.release();
    }
    return;
}


/*
    MOTOR FUNCTIONS
*/
Motor::Motor(int pinA, int pinB) {
    this.pinA = pinA;
    this.pinB = pinB;

    // this assumes the pybind interpreter has been initialized
    // in  APES::setup()!

    //@TODO: implement these for the L298N module!!!
    py::object l298n = py::module::import("libraries/l298n").attr("L298N");
    assert(l298n != NULL);

    this.HX711 = hx711(5, 6);
    this.HX711.attr("set_reading_format")("byte_format" _a="MSB", "bit_format" _a="MSB");
    this.HX711.attr("set_reference_unit")(1);
    this.HX711.attr("reset")();
    this.HX711.attr("tare")();
}

Motor::~Motor() {
    // this assumes the pybind interpreter will be
    // finalized in APES::finish()
    if (this.L298N != NULL) {
        // @TODO: implement finish func in the python module
        this.L298N.attr("finish")();
        this.L298N.release();
    }
    return;
}

void Motor::motor_drive(bool dir, int speed, int time) {
    //@TODO: implement the rest of this func
    if (this.L298N != NULL) {
        if (dir == 0) {
            this.L298N.attr("forward")(this.pinA, this.pinB);
        } else {
            this.L298N.attr("backward")(this.pinA, this.pinB);
        }
    }
}

void Motor::motor_stop() {
    if (this.L298N != NULL) {
        this.L298N.attr("stop")(this.pinA, this.pinB);
    }
}
