#ifndef APES_H
#define APES_H

#include "include/pybind11/embed.h"

class APES {
    private:
        pybind11::object HX711;
    public:
        APES();
        ~APES();
        int setup();
        int finish();
        void measWOB();
        int measMCP3008(int channel);
};

#endif
