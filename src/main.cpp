#include <pybind11/embed.h> // everything needed for embedding
namespace py = pybind11;

int main() {
    py::scoped_interpreter guard{}; // start the interpreter and keep it alive
    py::object module = py::module::import("example");//py::module::import("hx711").attr("HX711");
    //py::object hx = module("5, 6");
    
   // hx.


    
    return 0;
}
