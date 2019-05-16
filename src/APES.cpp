#include "APES.h"
#include <stdio.h>
#include <unistd.h>
#include <error.h>
#include <wiringPiSPI.h>

namespace py = pybind11;
using namespace py::literals;

APES::APES() {

}

APES::~APES() {

}

/* object methods */
int APES::setup() {
    
    // starts python interpreter
    py::initialize_interpreter();

    int motor_pinA = 1;
    int motor_pinB = 2;

    this.wob = new Wob();
    this.therm = new Therm();
    this.amm = new Amm();
    this.level = new Level();
    this.motor = new Motor(motor_pinA, motor_pinB);
    
    wiringPiSPISetup(0, 500000);
    wiringPiSPISetup(1, 500000);
    return 0;
}

float read_temp() {
    if (this.therm != NULL) {
        return this.therm->read_temp();
    }
    return -1;
}

float D_temp() {
    if (this.therm != NULL) {
        return this.therm->D_temp();
    }
    return -1;
}

float read_curr() {
    if (this.amm != NULL) {
        return this.amm->read_curr();
    }
    return -1;
}

int read_level() {
    if (this.level != NULL) {
        return this.level->read_level();
    }
    return -1;
}

void motor_drive(bool dir, int speed, int time) {
    if (this.motor != NULL) {
        this.motor->motor_drive(dir, speed, time);
    }
}

void motor_stop() {
    if (this.motor != NULL) {
        this.motor->motor_stop();
    }
}

int APES::standby() {
    // ensure that everything is off
    this.motor_stop();
    return 0;
}

int APES::finish() {
    // kills the python interpreter
    // and deletes everything, so
    // you better actually want to kill
    // the system if you call this func

    if (this.wob != NULL) { delete this.wob; }
    if (this.therm != NULL) { delete this.therm; }
    if (this.amm != NULL) { delete this.amm; }
    if (this.level != NULL) { delete this.level; }
    if (this.motor != NULL) {delete this.motor; }

    // super important that any pybind objects are
    // killed before pybind interpreter finalized!
    py::finalize_interpreter();
    return 0;
}

int APES::readData(const char *filename) {
    return -1;
}

int APES::writeData(float data, const char *filename) {
    FILE *file = fopen(filename, "a");
    if (file != NULL) {
        fprintf(file, "%f\n", data);
        fclose(file);
        return 0;
    } else {
        fprintf(stderr, "ERROR: %s", strerror(errno));
        return -1;
    }
}

int APES::writeData(int data, const char *filename) {
    FILE *file = fopen(filename, "a");
    if (file != NULL) {
        fprintf(file, "%d\n", data);
        fclose(file);
        return 0;
    } else {
        fprintf(stderr, "ERROR: %s", strerror(errno));
        return -1;
    }
}

