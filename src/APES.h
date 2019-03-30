#ifndef APES_H
#define APES_H

#include <pybind11/embed.h>

class APES {
    private:
        pybind11::object time;
        pybind11::object GPIO;
    public:
        pybind11::object HX711;
        APES();
        ~APES();
        void setup();
        void finish();
        void measWOB();
};

#endif
