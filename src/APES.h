#ifndef __APES_H__
#define __APES_H__

#include "include/pybind11/embed.h"
#include "analog.h"

class APES {
    private:
        wob_t HX711;
        therm_t thermo;
        amm_t ammeter;
        level_t wlevel; 
    public:
        APES();
        ~APES();
        int setup();
        int standby();
        int finish();
        void measWOB();
};

#endif
