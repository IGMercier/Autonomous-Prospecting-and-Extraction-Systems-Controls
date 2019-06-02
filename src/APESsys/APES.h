#ifndef _APES_H_
#define _APES_H_

#include <cstdio>
#include <vector>
#include <mutex>
#include <chrono>
#include "components.h"

#define MAXDATA 1024
#define SLEEP_INTVL 1000

// component pins
#define SOLENOID_0_PIN      29
#define SOLENOID_1_PIN      31
#define DC_HEATER_0_PIN     32
#define DC_HEATER_1_PIN     33
#define STEPPER_STP_PIN     36
#define STEPPER_DIR_PIN     35
//#define STEPPER_TIME_PIN  38
#define ENCODER_0_PIN       40
#define ENCODER_1_PIN       37
#define WOB_DATA_PIN        22
#define WOB_CLOCK_PIN       13
#define PUMP_SPEED_PIN      12
#define PUMP_DIR_PIN        15
#define DRILL_EN_PIN        11
#define DRILL_SPEED_PIN     7
#define RELAY_0_PIN         16
#define RELAY_1_PIN         18

#define THERM_CHAN          1
#define THERM_BUS           1
#define AMM_CHAN            2
#define AMM_BUS             1
#define WLEVEL_CHAN_START   0    
#define WLEVEL_CHAN_END     7
#define WLEVEL_BUS          0

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
        Motor* motor_X;
        Motor* motor_Y;
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
        void motor_X_drive(bool dir, int speed, int time);
        void motor_Y_drive(bool dir, int speed, int time);
        void motor_X_stop();
        void motor_Y_stop();
        void readData(const char *filename);
        void saveData(dataPt *data);
        void writeDataVector();
        void standby();
        void finish();
};

#endif
