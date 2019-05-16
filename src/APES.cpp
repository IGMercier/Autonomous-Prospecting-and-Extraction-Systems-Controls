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

    this->loadcell = make_wob();
    this->thermo = make_thermo();
    this->ammeter = make_amm();
    this->wlevel = make_level();

    wiringPiSPISetup(0, 500000);
    wiringPiSPISetup(1, 500000);

    return 0;
}

int APES::standby() {
    // ensure that everything is off
    return 0;
}

int APES::finish() {
    // kills the python interpreter
    py::finalize_interpreter();

    freeTherm(this->thermo);
    freeAmm(this->ammeter);
    freeLevel(this->wlevel);
    return 0;
}

void APES::measWOB() {
    py::object value;
    while (1) {
        value = this->HX711.attr("get_weight")("times"_a=5);
    

        printf("Got value: ");
        py::print(value);

        this->HX711.attr("power_down")();
        this->HX711.attr("power_up")();
        usleep(100000);
    }

    return;
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

