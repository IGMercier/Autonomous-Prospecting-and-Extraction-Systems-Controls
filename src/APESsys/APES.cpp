#include <thread>
#include <atomic>
#include <chrono>
#include <unistd.h>
#include <error.h>
#include "APES.h"
//#include "libraries/pybind11/include/pybind11/embed.h"

#define STEP_STEPS_TO_MS 1     // 1000 Hz

namespace py = pybind11;
using namespace py::literals;


APES::APES(char *filename, std::mutex *data_mtx) {
    assert(data_mtx != nullptr);

    if (filename != nullptr) {
        this->filename = filename;
    } else {
        this->filename = "data.csv";
    }
    
    FILE *file = fopen(this->filename, "w");
    fprintf(file, "time, sensor, value\n");
    fclose(file);

    this->data_mtx = data_mtx;

    this->sol_0 = nullptr;
    this->sol_1 = nullptr;
    this->heater_0 = nullptr;
    this->heater_1 = nullptr;
    this->relay_0 = nullptr;
    this->relay_1 = nullptr;
    this->drill = nullptr;
    this->wob = nullptr;
    this->encoder = nullptr;
    this->therm = nullptr;
    this->amm = nullptr;
    this->wlevel = nullptr;
    this->stepper = nullptr;
    this->pump = nullptr;
    this->spring = nullptr;

    // starts python interpreter
    py::initialize_interpreter();
}


/* object methods */
int APES::setup() {
    this->sol_0 = new Solenoid(SOLENOID_0_PIN);
    this->sol_1 = new Solenoid(SOLENOID_1_PIN);
    this->heater_0 = new DCHeater(DC_HEATER_0_PIN);
    this->heater_1 = new DCHeater(DC_HEATER_1_PIN);
    this->drill = new Drill(DRILL_EN_PIN, DRILL_SPEED_PIN);
    this->relay_0 = new Relay(RELAY_0_PIN);
    this->relay_1 = new Relay(RELAY_1_PIN);
    this->wob = new Wob(WOB_DATA_PIN, WOB_CLOCK_PIN);
    this->therm = new Therm(THERM_BUS, THERM_CHAN);
    this->amm = new Amm(AMM_BUS, AMM_CHAN);
    this->wlevel = new WLevel(WLEVEL_BUS, WLEVEL_CHAN_START, WLEVEL_CHAN_END);
    this->stepper = new Stepper(STEPPER_DIR_PIN, STEPPER_STP_PIN);
    this->pump = new Pump(PUMP_DIR_PIN, PUMP_SPEED_PIN);
    //this->spring = new Motor();

    int fd = wiringPiI2CSetup(ENCODER_ADDR);
    this->encoder = new Encoder(fd, 1024); // ppr from datasheet
    
    wiringPiSPISetup(0, 500000); // for adc
    wiringPiSPISetup(1, 500000); // for adc (level)
    wiringPiSetupPhys();
    pwmSetMode(PWM_MODE_MS);
    return 0;
}

dataPt* APES::read_temp() {
    if (this->therm != nullptr) {
        float temp = this->therm->read_temp();

        dataPt *data = new dataPt;
        data->time = std::chrono::high_resolution_clock::now().time_since_epoch().count();
        data->sensor = THERM_DATA;
        data->dataField.dataF = temp;
        
        std::unique_lock<std::mutex> datalock(*(this->data_mtx));
        saveData(data);
        datalock.unlock();

        return data;
    }
    return nullptr;
}

dataPt* APES::read_dtemp() {
    if (this->therm != nullptr) {
        float dtemp = this->therm->D_temp();

        dataPt *data = new dataPt;
        data->time = std::chrono::high_resolution_clock::now().time_since_epoch().count();
        data->sensor = THERM_DATA;
        data->dataField.dataF = dtemp;

        return data;
    }
    return nullptr;
}

dataPt* APES::read_curr() {
    if (this->amm != nullptr) {
        float curr = this->amm->read_curr();

        dataPt *data = new dataPt;
        data->time = std::chrono::high_resolution_clock::now().time_since_epoch().count();
        data->sensor = AMM_DATA;
        data->dataField.dataF = curr;
        
        std::unique_lock<std::mutex> datalock(*(this->data_mtx));
        saveData(data);
        datalock.unlock();

        return data;
    }
    return nullptr;
}

dataPt* APES::read_wlevel() {
    if (this->wlevel != nullptr) {
        int level = this->wlevel->read_wlevel();

        dataPt *data = new dataPt;
        data->time = std::chrono::high_resolution_clock::now().time_since_epoch().count();
        data->sensor = WLEVEL_DATA;
        data->dataField.dataI = level;
        
        std::unique_lock<std::mutex> datalock(*(this->data_mtx));
        saveData(data);
        datalock.unlock();

        return data;
    }
    return nullptr;
}

dataPt* APES::read_wob() {
    if (this->wob != nullptr) {
        float force = this->wob->read_wob();

        dataPt *data = new dataPt;
        data->time = std::chrono::high_resolution_clock::now().time_since_epoch().count();
        data->sensor = WOB_DATA;
        data->dataField.dataF = force;

        std::unique_lock<std::mutex> datalock(*(this->data_mtx));
        saveData(data);
        datalock.unlock();

        return data;
    }
    return nullptr;
}

dataPt* APES::read_encoder() {
    if (this->encoder != nullptr) {
        unsigned int pulse = this->encoder->getPulse();

        dataPt *data = new dataPt;
        data->time = std::chrono::high_resolution_clock::now().time_since_epoch().count();
        data->sensor = ENCODER_DATA;
        data->dataField.dataUI = pulse;
        
        std::unique_lock<std::mutex> datalock(*(this->data_mtx));
        saveData(data);
        datalock.unlock();

        return data;
    }
    return nullptr;
}

void APES::reset_encoder() {
    if (this->encoder != nullptr) {
        this->encoder->reset();
    }
}

void APES::drill_run(int dc, float freq) {
    if (this->drill != nullptr) {
        this->drill->drill_run(dc, freq);
    }
}

void APES::drill_stop() {
    if (this->drill != nullptr) {
        this->drill->drill_stop();
    }
}

void APES::drill_cycle(int dc, int on_period, float freq) {
    if (this->drill != nullptr) {
        this->drill->drill_cycle(dc, on_period, freq);
    }
}

void APES::sol_0_open() {
    if (this->sol_0 != nullptr) {
        this->sol_0->openValve();
    }
}

void APES::sol_0_close() {
    if (this->sol_0 != nullptr) {
        this->sol_0->closeValve();
    }
}

void APES::sol_1_open() {
    if (this->sol_1 != nullptr) {
        this->sol_1->openValve();
    }
}

void APES::sol_1_close() {
    if (this->sol_1 != nullptr) {
        this->sol_1->closeValve();
    }
}

void APES::heater_0_on() {
    if (this->heater_0 != nullptr) {
        this->heater_0->turnOn();
    }
}

void APES::heater_0_off() {
    if (this->heater_0 != nullptr) {
        this->heater_0->turnOff();
    }
}

void APES::heater_1_on() {
    if (this->heater_1 != nullptr) {
        this->heater_1->turnOn();
    }
}

void APES::heater_1_off() {
    if (this->heater_1 != nullptr) {
        this->heater_1->turnOff();
    }
}

void APES::relay_0_on() {
    if (this->relay_0 != nullptr) {
        this->relay_0->turnOn();
    }
}

void APES::relay_0_off() {
    if (this->relay_0 != nullptr) {
        this->relay_0->turnOff();
    }
}

void APES::relay_1_on() {
    if (this->relay_1 != nullptr) {
        this->relay_1->turnOn();
    }
}

void APES::relay_1_off() {
    if (this->relay_1 != nullptr) {
        this->relay_1->turnOff();
    }
}

void APES::stepper_drive(int steps, float freq) {
    if (this->stepper != nullptr) {
	    dataPt *prev = read_encoder();

        this->stepper->stepper_drive(steps, freq);

        dataPt *curr = read_encoder();

        dataPt *diffPt = new dataPt;
        diffPt->time = curr->time;
        diffPt->sensor = ENCODER_DIFF;

        int diff = (int)(curr->dataField.dataUI - prev->dataField.dataUI);
        diffPt->dataField.dataI = diff;
        
        std::unique_lock<std::mutex> datalock(*(this->data_mtx));
        saveData(diffPt);
        datalock.unlock();

        /*
	    std::chrono::time_point<std::chrono::high_resolution_clock> time;

	    int steps_time = steps * STEP_STEPS_TO_MS;
	    dataPt *previous = read_encoder();
	    time = std::chrono::high_resolution_clock::now() + std::chrono::milliseconds(steps_time);

	    this->stepper->stepper_drive(dir);

	    while(std::chrono::high_resolution_clock::now() < time);
	    this->stepper->stepper_stop();

	    dataPt *current = read_encoder();

        int actual = (int)(current->dataField.dataUI - previous->dataField.dataUI);
	
	    dataPt *data = new dataPt;
        data->time = std::chrono::high_resolution_clock::now().time_since_epoch().count();
        data->sensor = ENCODER_DIFF;
        data->dataField.dataI = actual;
        
        std::unique_lock<std::mutex> datalock(*(this->data_mtx));

        saveData(data);
        datalock.unlock();
        */
    }
}

void APES::stepper_stop() {
    if (this->stepper != nullptr) {
        this->stepper->stepper_stop();
    }
}

void APES::pump_drive(int dir, int dc) {
    if (this->pump != nullptr) {
        this->pump->pump_drive(dir, dc);
    }
}

void APES::pump_stop() {
    if (this->pump != nullptr) {
        this->pump->pump_stop();
    }
}

void APES::spring_drive(int dir, int dc) {
    if (this->spring != nullptr) {
        //this->spring->motor_drive(dir, speed, time);
    }
}

void APES::spring_stop() {
    if (this->spring != nullptr) {
        //this->spring->motor_stop();
    }
}

void APES::standby() {

    drill_stop();
    stepper_stop();
    pump_stop();
    heater_0_off();
    heater_1_off();
    spring_stop();
    sol_0_close();
    sol_1_close();
    relay_0_off();
    relay_1_off();
}

void APES::finish() {
    // stop all actuators before deconstructor
    standby();

    // kills the python interpreter
    // and deletes everything, so
    // you better actually want to kill
    // the system if you call this func
    std::unique_lock<std::mutex> datalock(*(this->data_mtx));
    writeDataVector();
    for (unsigned int i = 0; i < this->dataVector.size(); i++) {
        delete this->dataVector.at(i);
    }
    this->dataVector.clear();
    datalock.unlock();

    if (this->therm != nullptr) { delete this->therm; }
    if (this->amm != nullptr) { delete this->amm; }
    if (this->wlevel != nullptr) { delete this->wlevel; }
    if (this->wob != nullptr) { delete this->wob; }
    if (this->encoder != nullptr) { delete this->encoder; }
    if (this->stepper != nullptr) { delete this->stepper; }
    if (this->pump != nullptr) { delete this->pump; }
    if (this->drill != nullptr) { delete this->drill; }
    if (this->spring != nullptr) { delete this->spring; }
    if (this->sol_0 != nullptr) { delete this->sol_0; }
    if (this->sol_1 != nullptr) { delete this->sol_1; }
    if (this->heater_0 != nullptr) { delete this->heater_0; }
    if (this->heater_1 != nullptr) { delete this->heater_1; }
    if (this->relay_0 != nullptr) { delete this->relay_0; }
    if (this->relay_1 != nullptr) { delete this->relay_1; }

    // super important that any pybind objects are
    // killed before pybind interpreter finalized!
    py::finalize_interpreter();
}

void APES::saveData(dataPt *data) {
    assert(data != nullptr);
    // assumed that data_mtx is locked

    if (this->dataVector.size() >= MAXDATA) {
        writeDataVector();
    }
    this->dataVector.push_back(data);

    return;
}

void APES::writeDataVector() {
    // assumed that data_mtx is locked
    
    FILE *file = fopen(this->filename, "a");
    for (unsigned int i = 0; i < this->dataVector.size(); i++) {
        dataPt *data = this->dataVector.at(i);

        switch(data->sensor) {
            case THERM_DATA:
                fprintf(file, "%lli, %s, %f\n",
                        data->time, "therm", data->dataField.dataF);
                break;
            case AMM_DATA:
                fprintf(file, "%lli, %s, %f\n",
                        data->time, "amm", data->dataField.dataF);
                break;
            case WLEVEL_DATA:
                fprintf(file, "%lli, %s, %d\n",
                        data->time, "wlevel", data->dataField.dataI);
                break;
            case WOB_DATA:
                fprintf(file, "%lli, %s, %f\n",
                        data->time, "wob", data->dataField.dataF);
                break;
            case ENCODER_DATA:
                fprintf(file, "%lli, %s, %u\n",
                        data->time, "encoder", data->dataField.dataUI);
                break;
            case ENCODER_DIFF:
                fprintf(file, "%lli, %s, %u\n",
                        data->time, "encoder-diff", data->dataField.dataUI);
                break;
            default:
                fprintf(file, "%lli, %s, %f\n",
                        data->time, "none", data->dataField.dataF);
                break;
        }

        delete data;
    }
    this->dataVector.clear();
    fclose(file);
    return;
}

APES::~APES() {
}
