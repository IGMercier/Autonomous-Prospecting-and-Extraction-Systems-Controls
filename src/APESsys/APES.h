#ifndef _APES_H_
#define _APES_H_

#include <cstdio>
#include <vector>
#include <mutex>
#include <chrono>
#include "components.h"

#define MAXDATA 1024
#define SLEEP_INTVL 1000

typedef enum {
    AUTO_THERM = 0b1,
    AUTO_AMM = 0b10,
    AUTO_WLEVEL = 0b100,
    AUTO_WOB = 0b1000,
    AUTO_ALL = 0b10000,
    AUTO_NONE = 0b100000
} autoFunc;

typedef enum {
    THERM_DATA,
    AMM_DATA,
    WLEVEL_DATA,
    WOB_DATA,
    ENCODER_DATA
} sensor;

typedef struct dataPt {
    sensor origin;
    union {
        int dataI;
        float dataF;
    } dataField;

    std::chrono::time_point<std::chrono::system_clock> time;
} dataPt;

class APES {
    private:
        char *filename;
        std::mutex *data_mtx;
        std::vector<dataPt *> dataVector;
        Wob* wob;
        Therm* therm;
        Amm* amm;
        WLevel* wlevel;
        Motor* motor;
        Encoder *encoder;
    public:
        APES();
        ~APES();
        int setup(char *filename);
        dataPt* read_temp();
        dataPt* read_dtemp();
        dataPt* read_curr();
        dataPt* read_wlevel();
        dataPt* read_wob();
        dataPt* read_encoder();
        void auto_on(autoFunc which);
        void auto_off(autoFunc which);
        void motor_drive(bool dir, int speed, int time);
        void motor_stop();
        void readData(const char *filename);
        void saveData(dataPt *data);
        void writeDataVector();
        void standby();
        void finish();
};

#endif
