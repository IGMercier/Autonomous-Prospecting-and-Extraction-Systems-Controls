#ifndef __APES_H__
#define __APES_H__

#include "include/pybind11/embed.h"
#include "components.h"

class APES {
    private:
        Wob* wob;
        Therm* therm;
        Amm* amm;
        Level* level;
        Motor* motor;
        int isSetup = 0;
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
        int writeData(float data,/* time?, */ const char *filename);
        int writeData(int data,/* time?, */ const char *filename);
        int standby();
        int finish();
};

#endif
