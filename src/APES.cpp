#include "APES.h"

namespace py = pybind11;

/* object methods */
void APES::setup() {
    py::scoped_interpreter guard{}
    py::object hx711_module = py::module::import("hx711").attr("HX711");
    this.time = py::module::import("time");
    this.GPIO = py::module::import("RPIO.GPIO");

    // instantiates HX711 object
    this.HX711 = hx711_module("5, 6");

    // setup of HX711 module
    this.HX711.attr("set_reading_format")("'MSB', 'MSB;");
    this.HX711.attr("set_reference_unit")("1");
    this.HX711.attr("reset")();
    this.HX711.attr("tare")();
    py::print("Tare done! Add weight now!");
}

void APES::finish() {
    py::print("Cleaning...!");
    this.GPIO.attr("cleanup")();
    py::print("Exiting...!");
}

void APES::measWOB() {
    py::object value;
    while (1) {
        value = this.HX711.attr("get_weight")("5");
        py::print(value);

        this.HX711.attr("power_down")();
        this.HX711.attr("power_up")();
       
        this.time.attr("sleep")("0.1");
    }
}

