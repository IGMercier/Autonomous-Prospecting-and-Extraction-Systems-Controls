#ifndef APES_H
#define APES_H

#include <pybind11/embed.h>

class APES {
    private:
        pybind11::object GPIO;
        pybind11::object HX711;
    public:
        APES();
        ~APES();
        void setup();
        void finish();
        void measWOB();
        int measMCP3008(int channel);
};

#endif
