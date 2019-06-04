#include <thread>
#include <atomic>
#include <assert>
#include <unistd.h>
#include <error.h>
#include "APES.h"
//#include "libraries/pybind11/include/pybind11/embed.h"

namespace py = pybind11;
using namespace py::literals;

std::atomic_int stop_therm = {0};
std::atomic_int stop_amm = {0};
std::atomic_int stop_wlevel = {0};
std::atomic_int stop_wob = {0};
std::atomic_int stop_encoder = {0};

APES::APES(char *filename, std::mutex *data_mtx) {
    if (filename != NULL) {
        this->filename = filename;
    } else {
        this->filename = "data.csv";
    }
    
    FILE *file = fopen(this->filename, "w");
    fprintf(file, "time, sensor, value\n");
    fclose(file);

    this->data_mtx = data_mtx;

    this->sol_0 = NULL;
    this->sol_1 = NULL;
    this->heater_0 = NULL;
    this->heater_1 = NULL;
    this->relay_0 = NULL;
    this->relay_1 = NULL;
    this->drill = NULL;
    this->wob = NULL;
    this->therm = NULL;
    this->amm = NULL;
    this->wlevel = NULL;
    this->stepper = NULL;
    this->pump = NULL;
    this->encoder = NULL;

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

    int fd = wiringPiI2CSetup(ENCODER_ADDR);
    this->encoder = new Encoder(fd, 1024); // ppr from datasheet
    
    wiringPiSPISetup(0, 500000); // for adc
    wiringPiSPISetup(1, 500000); // for adc (level)
    wiringPiSetupPhys();
    pwmSetMode(PWM_MODE_MS);
    return 0;
}

dataPt* APES::read_temp() {
    if (this->therm != NULL) {
        float temp = this->therm->read_temp();

        dataPt *data = new dataPt;
        data->time = std::chrono::system_clock::now();
        data->sensor = THERM_DATA;
        data->dataField.dataF = temp;
        
        std::unique_lock<std::mutex> datalock(*(this->data_mtx));
        saveData(data);
        datalock.unlock();

        return data;
    }
    return NULL;
}

dataPt* APES::read_dtemp() {
    if (this->therm != NULL) {
        float dtemp = this->therm->D_temp();

        dataPt *data = new dataPt;
        data->time = std::chrono::system_clock::now();
        data->sensor = THERM_DATA;
        data->dataField.dataF = dtemp;

        return data;
    }
    return NULL;
}

dataPt* APES::read_curr() {
    if (this->amm != NULL) {
        float curr = this->amm->read_curr();

        dataPt *data = new dataPt;
        data->time = std::chrono::system_clock::now();
        data->sensor = AMM_DATA;
        data->dataField.dataF = curr;
        
        std::unique_lock<std::mutex> datalock(*(this->data_mtx));
        saveData(data);
        datalock.unlock();

        return data;
    }
    return NULL;
}

dataPt* APES::read_wlevel() {
    if (this->wlevel != NULL) {
        int level = this->wlevel->read_wlevel();

        dataPt *data = new dataPt;
        data->time = std::chrono::system_clock::now();
        data->sensor = WLEVEL_DATA;
        data->dataField.dataI = level;
        
        std::unique_lock<std::mutex> datalock(*(this->data_mtx));
        saveData(data);
        datalock.unlock();

        return data;
    }
    return NULL;
}

dataPt* APES::read_wob() {
    if (this->wob != NULL) {
        float force = this->wob->read_wob();

        dataPt *data = new dataPt;
        data->time = std::chrono::system_clock::now();
        data->sensor = WOB_DATA;
        data->dataField.dataF = force;

        std::unique_lock<std::mutex> datalock(*(this->data_mtx));
        saveData(data);
        datalock.unlock();

        return data;
    }
    return NULL;
}

dataPt* APES::read_encoder() {
    if (this->encoder != NULL) {
        unsigned int pulse = this->encoder->getPulse();

        dataPt *data = new dataPt;
        data->time = std::chrono::system_clock::now();
        data->sensor = ENCODER_DATA;
        data->dataField.dataUI = pulse;
        
        std::unique_lock<std::mutex> datalock(*(this->data_mtx));
        saveData(data);
        datalock.unlock();

        return data;
    }
    return NULL;
}

void APES::reset_encoder() {
    if (this->encoder != NULL) {
        this->encoder->reset();
    }
}

void APES::drill_run(int dc, float freq) {
    if (this->drill != NULL) {
        this->drill->drill_run(dc, freq);
    }
}

void APES::drill_stop() {
    if (this->drill != NULL) {
        this->drill->drill_stop();
    }
}

void APES::drill_cycle(int dc, int on_period, float freq) {
    if (this->drill != NULL) {
        this->drill->drill_cycle(dc, on_period, freq);
    }
}

void APES::sol_0_open() {
    if (this->sol_0 != NULL) {
        this->sol_0->openValve();
    }
}

void APES::sol_0_close() {
    if (this->sol_0 != NULL) {
        this->sol_0->closeValve();
    }
}

void APES::sol_1_open() {
    if (this->sol_1 != NULL) {
        this->sol_1->openValve();
    }
}

void APES::sol_1_close() {
    if (this->sol_1 != NULL) {
        this->sol_1->closeValve();
    }
}

void APES::heater_0_on() {
    if (this->heater_0 != NULL) {
        this->heater_0->turnOn();
    }
}

void APES::heater_0_off() {
    if (this->heater_0 != NULL) {
        this->heater_0->turnOff();
    }
}

void APES::heater_1_on() {
    if (this->heater_1 != NULL) {
        this->heater_1->turnOn();
    }
}

void APES::heater_1_off() {
    if (this->heater_1 != NULL) {
        this->heater_1->turnOff();
    }
}

void APES::relay_0_on() {
    if (this->relay_0 != NULL) {
        this->relay_0->turnOn();
    }
}

void APES::relay_0_off() {
    if (this->relay_0 != NULL) {
        this->relay_0->turnOff();
    }
}

void APES::relay_1_on() {
    if (this->relay_1 != NULL) {
        this->relay_1->turnOn();
    }
}

void APES::relay_1_off() {
    if (this->relay_1 != NULL) {
        this->relay_1->turnOff();
    }
}

void APES::stepper_drive(bool dir, int steps, int dc) {

    // @TODO: integer overflow will happen here
    if (this->stepper != NULL) {
        unsigned int actual = read_encoder().dataUI;
        unsigned int desired = steps + actual;
        while (actual < desired) {
            this->stepper->stepper_drive(dir, dc);
            actual = read_encoder().dataUI;
        }
        if (actual > desired) { stepper_stop(); }
    }
}

void APES::stepper_stop() {
    if (this->stepper != NULL) {
        this->stepper->stepper_stop();
    }
}

void APES::pump_drive(bool dir, int speed, int time) {
    if (this->pump != NULL) {
        this->pump->pump_drive(dir, speed, time);
    }
}

void APES::pump_stop() {
    if (this->pump != NULL) {
        this->pump->pump_stop();
    }
}

void APES::auto_on() {
    stop_therm.store(0);
    stop_amm.store(0);
    stop_wlevel.store(0);
    stop_wob.store(0);
    stop_encoder.store(0);

    while (1) {
       
        if (!stop_therm.load()) {
            read_temp();
        }
        
        if (!stop_amm.load()) {
            read_amm();
        }

        if (!stop_wlevel.load()) {
            read_wlevel();
        }

        if (!stop_wob.load()) {
            read_wob();
        }

        if (!stop_encoder.load()) {
            read_encoder();
        }

        if (stop_therm.load() && stop_amm.load() &&
            stop_wlevel.load() && stop_wob.load() &&
            stop_encoder.load()) {
            return;
        }

        std::this_thread::sleep_for(
            std::chrono::milliseconds(SLEEP_INTVL)
        );
    }
}

void APES::auto_off() {
        stop_therm.store(1);
        stop_amm.store(1);
        stop_wlevel.store(1);
        stop_wob.store(1);
        stop_encoder.store(1);
    return;
}

void APES::standby() {
    // ensure that everything is off
    motor_Z_stop();
    pump_stop();
    stop_therm = 1;
    stop_amm = 1;
    stop_wlevel = 1;
    stop_wob = 1;
}

void APES::finish() {
    // stop all actuators before deconstructor
    stop_therm.store(1);
    stop_amm.store(1);
    stop_wlevel.store(1);
    stop_wob.store(1);
    stop_encoder.store(1);

    stepper_stop();
    pump_stop();

    // kills the python interpreter
    // and deletes everything, so
    // you better actually want to kill
    // the system if you call this func
    std::unique_lock<std::mutex> datalock(*(this->data_mtx));
    writeDataVector();
    for (unsigned int  = 0; i < this->dataVector.size(); i++) {
        delete this->dataVector.at(i);
    }
    this->dataVector.clear();
    fclose(this->file);
    datalock.unlock();

    if (this->wob != NULL) { delete this->wob; }
    if (this->therm != NULL) { delete this->therm; }
    if (this->amm != NULL) { delete this->amm; }
    if (this->wlevel != NULL) { delete this->wlevel; }
    if (this->motor_Z != NULL) { delete this->motor_Z; }
    if (this->pump != NULL) { delete this->pump; }

    // super important that any pybind objects are
    // killed before pybind interpreter finalized!
    py::finalize_interpreter();
}

void APES::saveData(dataPt *data) {
    assert(data != NULL);
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

        switch(data->origin) {
            case THERM_DATA:
                fprintf(file, "%li, %s, %f\n",
                        data->time.count(), "therm", data->dataField.dataF);
                break;
            case AMM_DATA:
                fprintf(file, "%li, %s, %f\n",
                        data->time.count(), "amm", data->dataField.dataF);
                break;
            case WLEVEL_DATA:
                fprintf(file, "%li, %s, %d\n",
                        data->time.count(), "wlevel", data->dataField.dataI);
                break;
            case WOB_DATA:
                fprintf(file, "%li, %s, %f\n",
                        data->time.count(), "wob", data->dataField.dataF);
                break;
            case ENCODER_DATA:
                fprintf(file, "%li, %s, %u\n",
                        data->time.count(), "encoder", data->dataField.dataUI);
                break;
            default:
                fprintf(file, "%li, %s, %f\n",
                        data->time.count(), "none", data->dataField.dataF);
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
