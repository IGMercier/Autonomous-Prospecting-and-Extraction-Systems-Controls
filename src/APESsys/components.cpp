#include <chrono>
#include <cstdio>
#include <cstdlib>

#define _USE_MATH_DEFINES
#include <cmath>

#include "components.h"

#define THERM_RATE 0.259
#define THERM_OFFSET -37.0
#define AMM_RATE 3.24
#define AMM_OFFSET -20.64

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
        fprintf(stderr, "ERROR: thermometer instantiation failed!\n");
        return;
    }

    if ((chan > 7) || (chan  < 0)) {
        fprintf(stderr, "ERROR: thermometer channel address takes values 0-7!\n");
        fprintf(stderr, "ERROR: thermometer instantiation failed!\n");
        return;
    }

    this->bus = bus;
    this->chan = chan;
    this->iTemp = read_temp();
    fprintf(stdout, "Initialized Therm!\n");
}

float Therm::read_temp() {
    return THERM_RATE * (float)readADC(this->bus, this->chan) + THERM_OFFSET;
}

float Therm::D_temp() {
    return read_temp() - this->iTemp;
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
    return AMM_RATE * (float)readADC(this->bus, this->chan) + AMM_OFFSET;
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
    if (chan_start > chan_end) {
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
    ENCODER FUNCTIONS
*/
Encoder::Encoder(int fd, int ppr) {
    this->fd = fd;
    this->ppr = ppr;
}

Encoder::~Encoder() {}

unsigned int Encoder::getPulse() {
    return (unsigned int)wiringPiI2CRead(this->fd);
}

void Encoder::reset() {
    wiringPiI2CWrite(this->fd, 0);
}

float Encoder::calcVel(int n, int t) {
    // n = number of pulses
    // t = sampling time, should be constant (?)
    // @TODO: figure out these params
    return (float)(2*M_PI*n / (this->ppr * t));
}


/*
    DRILL FUNCTIONS
*/
Drill::Drill(int en_pin, int pwm_pin) {
    this->en_pin = en_pin;
    this->pwm_pin = pwm_pin;
    pinMode(this->en_pin, OUTPUT);
    pinMode(this->pwm_pin, PWM_OUTPUT);
    digitalWrite(this->en_pin, LOW);
    pwmWrite(this->pwm_pin, LOW);
}

void Drill::drill_run(int dc, float freq) {
    float pwmClock = PWM_MAX_FREQ / freq / PWM_RANGE;
    pwmSetRange(PWM_RANGE);
    pwmSetClock(pwmClock);
    digitalWrite(this->en_pin, HIGH);
    pwmWrite(this->pwm_pin, (dc*1024/100));
}

void Drill::drill_stop() {
    digitalWrite(this->en_pin, LOW);
    pwmWrite(this->pwm_pin, LOW);
}

void Drill::drill_cycle(int dc, int on_period, float freq) {
    float time = dc / freq;
    
    auto start = std::chrono::high_resolution_clock::now();
    std::chrono::milliseconds elapsed{0};
    while (elapsed.count() < on_period) {
        auto start_dc = std::chrono::high_resolution_clock::now();
        std::chrono::milliseconds elapsed_dc{0};

        while ((elapsed_dc.count() < time) && (elapsed.count() < on_period)) {
            drill_run(dc, freq);
            auto stop_dc = std::chrono::high_resolution_clock::now();
            elapsed_dc = std::chrono::duration_cast<std::chrono::milliseconds>(stop_dc - start_dc);

            auto stop = std::chrono::high_resolution_clock::now();
            elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start);
        }
        drill_stop();
    }
}

Drill::~Drill() {}


/*
    STEPPER FUNCTIONS
*/
Stepper::Stepper(int dir_pin, int step_pin) {
    this->dir_pin = dir_pin;
    this->step_pin = step_pin;
    
    pinMode(this->dir_pin, OUTPUT);
    digitalWrite(this->dir_pin, LOW);

    softPwmCreate(this->step_pin, 0, 100);
}

void Stepper::stepper_drive(int dir, int dc, float freq) {
    float pwmClock = PWM_MAX_FREQ / freq / PWM_RANGE;
    pwmSetRange(PWM_RANGE);
    pwmSetClock(pwmClock);
    if (dir == 0) {
        digitalWrite(this->dir_pin, LOW);
    } else {
        digitalWrite(this->dir_pin, HIGH);
    }
    softPwmWrite(this->step_pin, dc);
}

void Stepper::stepper_stop() {
    softPwmWrite(this->step_pin, 0);
}

Stepper::~Stepper() {}


/*
    PUMP FUNCTIONS
*/
Pump::Pump(int dir_pin, int pwm_pin) {
    // this assumes the pybind interpreter has been initialized
    // in  APES::setup()!
    this->dir_pin = dir_pin;
    pinMode(this->dir_pin, OUTPUT);
    this->pwm_pin = pwm_pin;
    softPwmCreate(this->pwm_pin, 0, 100);
    fprintf(stdout, "Initialized Motor!\n");
}

Pump::~Pump() {}

void Pump::pump_drive(int dir, int dc) {
    if (dir == 0) {
        digitalWrite(this->dir_pin, HIGH);
    } else {
        digitalWrite(this->dir_pin, LOW);
    }
     softPwmWrite(this->pwm_pin, dc);
}

void Pump::pump_stop() {
    softPwmWrite(this->pwm_pin, 0);
}


/*
    SOLENOID FUNCTIONS
*/
Solenoid::Solenoid(int pin) {
    this->pin = pin;
    pinMode(this->pin, OUTPUT);
    digitalWrite(this->pin, LOW);
}

void Solenoid::openValve() {
    // assuming HIGH is to open
    digitalWrite(this->pin, HIGH);
}

void Solenoid::closeValve() {
    // assuming LOW is to close
    digitalWrite(this->pin, LOW);
}

Solenoid::~Solenoid() {}


/*
    DCHEATER FUNCTIONS
*/
DCHeater::DCHeater(int pin) {
    this->pin = pin;
    pinMode(this->pin, OUTPUT);
    digitalWrite(this->pin, LOW);
}

void DCHeater::turnOn() {
    digitalWrite(this->pin, HIGH);
}

void DCHeater::turnOff() {
    digitalWrite(this->pin, LOW);
}

DCHeater::~DCHeater() {}


/*
    RELAY FUNCTIONS
*/
Relay::Relay(int pin) {
    this->pin = pin;
    pinMode(this->pin, OUTPUT);
    digitalWrite(this->pin, LOW);
}

void Relay::turnOn() {
    digitalWrite(this->pin, HIGH);
}

void Relay::turnOff() {
    digitalWrite(this->pin, LOW);
}

Relay::~Relay() {}
