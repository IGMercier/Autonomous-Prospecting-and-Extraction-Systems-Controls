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
#define WOB_DATA_PIN        22
#define WOB_CLOCK_PIN       13
#define PUMP_SPEED_PIN      12
#define PUMP_DIR_PIN        15
#define DRILL_EN_PIN        11
#define DRILL_SPEED_PIN     7
#define RELAY_0_PIN         16
#define RELAY_1_PIN         18

#define THERM_CHAN          7
#define THERM_BUS           0
#define AMM_CHAN            6
#define AMM_BUS             0
#define WLEVEL_CHAN_START   0    
#define WLEVEL_CHAN_END     0
#define WLEVEL_BUS          0
#define ENCODER_ADDR        0x26 


typedef enum {
    THERM_DATA,
    AMM_DATA,
    WLEVEL_DATA,
    WOB_DATA,
    ENCODER_DATA,
    ENCODER_DIFF
} origin;

typedef struct dataPt {
    origin sensor;
    union {
        unsigned int dataUI; 
        int dataI;
        float dataF;
    } dataField;

    long long int time;
} dataPt;

class APES {
    private:
        char *filename;
        std::mutex *data_mtx;
        std::vector<dataPt *> dataVector;
        
        Solenoid* sol_0;
        Solenoid* sol_1;
        DCHeater* heater_0;
        DCHeater* heater_1;
        Stepper* stepper;
        Encoder *encoder;
        Wob* wob;
        Pump* pump;
        Motor* spring;
        Drill* drill;
        Relay* relay_0;
        Relay* relay_1;
        Therm* therm;
        Amm* amm;
        WLevel* wlevel;
    public:
        APES(char *filename, std::mutex *data_mtx);
        ~APES();
        int setup();
        dataPt* read_temp();
        dataPt* read_dtemp();
        dataPt* read_curr();
        dataPt* read_wlevel();
        dataPt* read_wob();
        dataPt* read_encoder();
        void reset_encoder();
        void auto_on();
        void auto_off();
        void drill_run(int dc, float freq);
        void drill_stop();
        void drill_cycle(int dc, int on_period, float freq);
        void stepper_drive(bool dir, int steps);
        void stepper_stop();
        void pump_drive(int dir, int speed, int time);
        void pump_stop();
        void spring_drive(int dir, int speed, int time);
        void spring_stop();
        void sol_0_open();
        void sol_0_close();
        void sol_1_open();
        void sol_1_close();
        void heater_0_on();
        void heater_0_off();
        void heater_1_on();
        void heater_1_off();
        void relay_0_on();
        void relay_0_off();
        void relay_1_on();
        void relay_1_off();
        void saveData(dataPt *data);
        void writeDataVector();
        void standby();
        void finish();
};

#endif
