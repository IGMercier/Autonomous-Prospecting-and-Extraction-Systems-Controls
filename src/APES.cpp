#include "APES.h"
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

    // imports python modules
   /* py::object hx711 = py::module::import("libraries/hx711").attr("HX711");
    if (hx711 == NULL) {
        fprintf(stderr, "Cannot import module hx711.py!\n");
	return -1;
    }

    // instantiates HX711 object
    this->HX711 = hx711(5, 6);
    if (this->HX711 == NULL) {
        fprintf(stderr, "Cannot instantiate object HX711!\n");
	return -1;
    }

    // setup of HX711 module
    this->HX711.attr("set_reading_format")("byte_format"_a="MSB", "bit_format"_a="MSB");
    this->HX711.attr("set_reference_unit")(1);
    this->HX711.attr("reset")();
    this->HX711.attr("tare")();
    printf("Tare done! Add weight now!\n");
    */
    wiringPiSPISetup(0, 500000);
    wiringPiSPISetup(1, 500000);

    return 0;
}

int APES::finish() {

    if (this->HX711 != NULL) {
        this->HX711.attr("finish")();
    
        // python objects cannot outlive
        // the interpreter!!! <- super important
        this->HX711.release();
    }

    // kills the python interpreter
    py::finalize_interpreter();
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

int APES::measMCP3008(int bus, int channel) {
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

