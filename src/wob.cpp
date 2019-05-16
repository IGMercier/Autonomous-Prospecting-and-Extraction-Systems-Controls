#include "wob.h"
#include <assert.h>

namespace py = pybind11;
using namespace py::literals;

wob_t make_wob() {
    // this assumes the pybind interpreter has been initialized
    // in  APES::setup()!
    wob_t loadcell = calloc(1, sizeof(wob));
    assert(loadcell != NULL);

    py::object hx711 = py::module::import("libraries/hx711").attr("HX711");
    assert(hx711 != NULL);

    loadcell->HX711 = hx711(5, 6);
    loadcell->HX711.attr("set_reading_format")("byte_format" _a="MSB", "bit_format" _a="MSB");
    loadcell->HX711.attr("set_reference_unit")(1);
    loadcell->HX711.attr("reset")();
    loadcell->HX711.attr("tare")();

    return loadcell;
}

float read_wob(wob_t loadcell) {
    py::object value;
    value = loadcell->HX711.attr("get_weight")("times" _a=5);
    loadcell->HX711.attr("power_down")();
    loadcell->HX711.attr("power_up")();

    //@TODO: look up how to convert pybind value to C++ primitive

    return 1;
}

void free_wob(wob_t loadcell) {
    // this assumes the pybind interpreter will be
    // finalized in APES::finish()
    if (loadcell->HX711 != NULL) {
        loadcell->HX711.attr("finish")();
        loadcell->HX711.release();
    }
    return;
}
