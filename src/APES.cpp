#include "APES.h"
#include <unistd.h>
#include <wiringPiSPI.h>

namespace py = pybind11;

APES::APES() {

}

APES::~APES() {

}

/* object methods */
void APES::setup() {
    
    // starts python interpreter
    py::scoped_interpreter guard{};

    // imports python modules
    py::object hx711 = py::module::import("hx711").attr("HX711");

    this->GPIO = py::module::import("RPi.GPIO");

    // instantiates HX711 object
    this->HX711 = hx711(5, 6);

    // setup of HX711 module
    this->HX711.attr("set_reading_format")("MSB, MSB");
    this->HX711.attr("set_reference_unit")(1);
    this->HX711.attr("reset")();
    this->HX711.attr("tare")();
    py::print("Tare done! Add weight now!");
    
    wiringPiSPISetup(0, 500000);

    return;
}

void APES::finish() {
    py::print("Cleaning...!");
    this->GPIO.attr("cleanup")();
    py::print("Exiting...!");

    return;
}

void APES::measWOB() {
    py::object value;
    while (1) {
        value = this->HX711.attr("get_weight")(5);
        py::print(value);

        this->HX711.attr("power_down")();
        this->HX711.attr("power_up")();
        usleep(100000);
    }

    return;
}

int APES::measMCP3008(int channel) {
    if (channel > 7) {
        return -1;
    }

    unsigned char data[3];
    data[0] = 0b1;
    data[1] = (0b1000 + channel) << 4;
    data[2] = 0;
    wiringPiSPIDataRW(0, data, 3);

    int datum = ((data[1] & 0b11) << 8) + data[2];

    return datum;
}

