#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <wiringPiSPI.h>

#define _USE_MATH_DEFINES
#include <cmath>

#include "components.h"

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
Therm::Therm(int bus, int chan) {
    if ((bus > 1) || (bus < 0)) {
        fprintf(stderr, "ERROR: thermometer bus address must be 0 or 1!\n");
        return;
    }

    if ((chan > 7) || (chan  < 0)) {
        fprintf(stderr, "ERROR: thermometer channel address takes values 0-7!\n");
        fprintf(stderr, "ERROR: thermometer instatiation failed!\n");
        return;
    }


    this->bus = bus;
    this->chan = chan;
    this->iTemp = 0; //read_temp();
    fprintf(stdout, "Initialized Therm!\n");
}

float Therm::read_temp() {
    return (float)readADC(this->bus, this->chan);
}

float Therm::D_temp() {
    float fTemp = read_temp();
    return fTemp - this->iTemp;
}

Therm::~Therm() {}


/*
    AMMETER FUNCTIONS
*/
Amm::Amm(int bus, int chan) {
    if ((bus > 1) || (bus < 0)) {
        fprintf(stderr, "ERROR: ammeter bus address must be 0 or 1!\n");
        return;
    }

    if ((chan > 7) || (chan  < 0)) {
        fprintf(stderr, "ERROR: ammeter channel address takes values 0-7!\n");
        fprintf(stderr, "ERROR: ammeter instatiation failed!\n");
        return;
    }

    this->bus = bus;
    this->chan = chan;
    fprintf(stdout, "Initialized Amm!\n");
}

float Amm::read_curr() {
    return (float)readADC(this->bus, this->chan);
}

Amm::~Amm() {}


/*
    WATER LEVEL FUNCTIONS
*/
WLevel::WLevel(int bus, int chan_start, int chan_end) {
    if ((bus > 1) || (bus < 0)) {
        fprintf(stderr, "ERROR: level bus address must be 0 or 1!\n");
        return;
    }
    if ((chan_start > 7) || (chan_start < 0)) {
        fprintf(stderr, "ERROR: level bus_start address takes values 0-7!\n");
        fprintf(stderr, "ERROR: level instatiation failed!\n");
        return;
    }
    if ((chan_end > 7) || (chan_end < 0)) {
        fprintf(stderr, "ERROR: level bus_end address takes values 0-7!\n");
        fprintf(stderr, "ERROR: level instatiation failed!\n");
        return;
    }

    // requires that chan_start <= chan_end
    if (bus_start > bus_end) {
        fprintf(stderr, "ERROR: level requires that bus_start address");
        fprintf(stderr, " <= bus_end address\n");
        fprintf(stderr, "ERROR: level instatiation failed!\n");
        return;
    }

    this->bus = bus;
    this->chan_start = chan_start;
    this->chan_end = chan_end;
    fprintf(stdout, "Initialized WLevel!\n");
}

int WLevel::read_wlevel() {
    /*
      reads from each ADC channel
      assumes 1 == water at that level
      assumes channel 0 is the lowest level
        and channel 7 is the highest level
    */

    int level = 0;
    int channel = this->chan_start;
    for ( ; channel < this->chan_end; channel++) {
        if (readADC(this->bus, channel) == 1) {
            //@TODO: should this be == 0 since
            // the reading might aren't a binary {0, 1}?
            level = channel;
        }
    }

    return level;
}

WLevel::~WLevel() {}


/*
    WOB FUNCTIONS
*/
Wob::Wob(int pinA, int pinB) {
    // this assumes the pybind interpreter has been initialized
    // in  APES::setup()!
    py::object hx711 = py::module::import("libraries.hx711py.hx711").attr("HX711");
    assert(hx711 != NULL);

    this->HX711 = hx711(pinA, pinB);
    //this->HX711.attr("set_reading_format")("byte_format"_a="MSB", "bit_format"_a="MSB");
    //this->HX711.attr("set_reference_unit")(1);
    //this->HX711.attr("reset")();
    //this->HX711.attr("tare")();
    fprintf(stdout, "Initialized Wob!\n");
}

float Wob::read_wob() {
    py::object value;
    value = this->HX711.attr("get_weight")("times"_a=5);
    this->HX711.attr("power_down")();
    this->HX711.attr("power_up")();

    float force = value.cast<float>();

    return force;
}

Wob::~Wob() {
    // this assumes the pybind interpreter will be
    // finalized in APES::finish()
    if (this->HX711 != NULL) {
        // @TODO: implement clean func in the python module
        this->HX711.attr("clean")();
        this->HX711.release();
    }
}


/*
    MOTOR FUNCTIONS
*/
Motor::Motor(int pinA, int en) {
    // this assumes the pybind interpreter has been initialized
    // in  APES::setup()!
    py::object l298n = py::module::import("libraries/l298npy/l298n").attr("L298N");
    assert(l298n != NULL);

    this->L298N = l298n(pinA, en);
    fprintf(stdout, "Initialized Motor!\n");
}

Motor::~Motor() {
    // this assumes the pybind interpreter will be
    // finalized in APES::finish()
    if (this->L298N != NULL) {
        this->L298N.attr("clean")();
        this->L298N.release();
    }
}

void Motor::motor_drive(bool dir, int speed, int time) {
    // time in milliseconds
    //@TODO: really need to test this!
    if (this->L298N != NULL) {
        //@TODO: calculate an actual duty cycle from speed?
        this->L298N.attr("changeDutyCycle")("dc"_a=speed);

        auto start = std::chrono::high_resolution_clock::now();
        std::chrono::milliseconds elapsed{0}; 
        while (elapsed.count() < time){
            if (dir == 0) {
                this->L298N.attr("forward")();
            } else {
                this->L298N.attr("backward")();
            }

            auto stop = std::chrono::high_resolution_clock::now();
            elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start);
        }
    }
}

void Motor::motor_stop() {
    if (this->L298N != NULL) {
        this->L298N.attr("stop")();
    }
}


/*
    ENCODER FUNCTIONS
*/
Encoder::Encoder(int fd, int ppr) {
    this->fd = fd;
    this->ppr = ppr;
}

Encoder::~Encoder() {

}

Encoder::getTicks() {
    /*
        @TODO: this literally does nothing,
               its just here to show how to read and write
    */
    wiringPiI2CWrite(this->fd, -1);
    wiringPiI2CRead(this->fd);
}

Encoder::reset() {
    /*
        @TODO: this literally does nothing,
               its just here to show how to read and write
    */
    wiringPiI2CWrite(this->fd, -1);
    wiringPiI2CRead(this->fd);

}

float Encoder::calcVel(int n, int t) {
    // n = number of pulses
    // t = sampling time, should be constant (?)
    // @TODO: figure out these params
    return (float)(2*M_PI*n / (this->ppr * t));
}
