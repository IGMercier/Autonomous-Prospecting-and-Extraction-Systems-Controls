#ifndef __APES_H__
#define __APES_H__

//#include "libraries/pybind11/include/pybind11/embed.h" <- I might not need this here!
#include "components.h"

#define MAXDATA 1024

class APES {
    private:
        char *filename;
        std::FILE *file;
        std::vector<dataPt *> dataArray;
        Wob* wob;
        Therm* therm;
        Amm* amm;
        Level* level;
        Motor* motor;
    public:
        APES();
        ~APES();
        int setup(std::string filename);
        float read_temp();
        float D_temp();
        float read_curr();
        int read_level();
        void motor_drive(bool dir, int speed, int time);
        void motor_stop();
        int readData(const char *filename);
        int writeData(dataPt *data);
        int standby();
        int finish();
};

#endif
