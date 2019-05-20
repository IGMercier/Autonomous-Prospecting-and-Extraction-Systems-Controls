#ifndef __APES_H__
#define __APES_H__

//#include "libraries/pybind11/include/pybind11/embed.h" <- I might not need this here!

#include <cstdio>
#include <vector>
#include "components.h"

#define MAXDATA 1024

class APES {
    private:
        char *filename;
        std::FILE *file;
        std::vector<dataPt *> dataVector;
        Wob* wob;
        Therm* therm;
        Amm* amm;
        WLevel* wlevel;
        Motor* motor;
    public:
        APES();
        ~APES();
        int setup(char *filename);
        float read_temp();
        float D_temp();
        float read_curr();
        int read_wlevel();
        void motor_drive(bool dir, int speed, int time);
        void motor_stop();
        void readData(const char *filename);
        void saveData(dataPt *data);
        void writeDataVector();
        void standby();
        void finish();
};

#endif
