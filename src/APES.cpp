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

    this.wob = new Wob();
    this.therm = new Therm();
    this.amm = new Amm();
    this.level = new Level();
    
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

int APES::standby() {
    // ensure that everything is off
    return 0;
}

int APES::finish() {
    // kills the python interpreter
    // and deletes everything, so
    // you better actually want to kill
    // the robot if you call this func

    // super important that any pybind objects are
    // killed before pybind interpreter finalized!
    if (this.wob != NULL) { delete this.wob; }
    if (this.therm != NULL) { delete this.therm; }
    if (this.amm != NULL) { delete this.amm; }
    if (this.level != NULL) { delete this.level; }
    py::finalize_interpreter();
    return 0;
}

int APES::writeData(float data, const char *filename) {
    FILE *file = fopen(filename, "a");
    if (file != NULL) {
        fprintf(file, "%f\n", data);
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
        return 0;
    } else {
        fprintf(stderr, "ERROR: %s", strerror(errno));
        return -1;
    }
}

