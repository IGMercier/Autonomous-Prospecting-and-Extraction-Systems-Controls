#ifndef __APES_H__
#define __APES_H__

//#include "libraries/pybind11/include/pybind11/embed.h" <- I might not need this here!
#include "components.h"

class APES {
    private:
        Wob* wob;
        Therm* therm;
        Amm* amm;
        Level* level;
        Motor* motor;
    public:
        APES();
        ~APES();
        int setup();
        float read_temp();
        float D_temp();
        float read_curr();
        int read_level();
        void motor_drive(bool dir, int speed, int time);
        void motor_stop();
        int readData(const char *filename);
        int writeDataFloat(float data,/* time?, */ const char *filename);
        int writeDataInt(int data,/* time?, */ const char *filename);
        int standby();
        int finish();
};

#endif
