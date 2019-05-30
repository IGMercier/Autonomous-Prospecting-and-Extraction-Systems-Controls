#include <thread>
#include <atomic>
#include <wiringPiSPI.h>
#include <assert>
#include <unistd.h>
#include <error.h>
#include "APES.h"
#include "libraries/pybind11/include/pybind11/embed.h"

namespace py = pybind11;
using namespace py::literals;

static void therm_thread(APES *robot);
static void amm_thread(APES *robot);
static void wlevel_thread(APES *robot);
static void wob_thread(APES *robot);

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

    this->wob = NULL;
    this->therm = NULL;
    this->amm = NULL;
    this->wlevel = NULL;
    this->motor = NULL;


    // starts python interpreter
    py::initialize_interpreter();
}


/* object methods */
int APES::setup() {
    //@TODO: put the correct pin numbers 
    this->wob = new Wob(2, 3);
    this->therm = new Therm(7, 27);
    this->amm = new Amm(5, 6);
    this->wlevel = new WLevel(2, 4);
    this->motor = new Motor(26, 8);

    int fd = wiringPiI2CSetup(/* fill with device id*/);
    this->encoder = new Encoder(fd, 1024); // ppr from datasheet
    
    wiringPiSPISetup(0, 500000); // for adc
    wiringPiSPISetup(1, 500000); // for adc (level)
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
        int force = this->wob->read_wob();

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

void APES::motor_drive(bool dir, int speed, int time) {
    if (this->motor != NULL) {
        this->motor->motor_drive(dir, speed, time);
    }
}

void APES::motor_stop() {
    if (this->motor != NULL) {
        this->motor->motor_stop();
    }
}

void APES::auto_on() {
    stop_therm = 0;
    stop_amm = 0;
    stop_wlevel = 0;
    stop_wob = 0;

    std::thread thermt(therm_thread, this);
    std::thread ammt(amm_thread, this);
    std::thread wlevelt(wlevel_thread, this);
    std::thread wobt(wob_thread, this);

    if (thermt.joinable()) {
        thermt.join();
    }
    if (ammt.joinable()) {
        ammt.join();
    }
    if (wlevelt.joinable()) {
        wlevelt.join();
    }
    if (wobt.joinable()) {
        wobt.join();
    }
    return;
}

void APES::auto_off() {
    stop_therm = 1;
    stop_amm = 1;
    stop_wlevel = 1;
    stop_wob = 1;
    return;
}

void APES::standby() {
    // ensure that everything is off
    motor_stop();
    stop_therm = 1;
    stop_amm = 1;
    stop_wlevel = 1;
    stop_wob = 1;
}

void APES::finish() {
    // stop all actuators before deconstructor
    stop_therm = 1;
    stop_amm = 1;
    stop_wlevel = 1;
    stop_wob = 1;

    motor_stop();

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
    if (this->motor != NULL) { delete this->motor; }

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
                fprintf(file, "%li, %s, %f\n",
                        data->time.count(), "encoder", data->dataField.dataF);
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

static void therm_thread(APES *robot) {
    assert(robot != NULL);

    while (!stop_therm) {
        // @TODO: test this
        dataPt *data = robot->read_temp();
    }

    return;
}

static void amm_thread(APES *robot) {
    assert(robot != NULL);

    while (!stop_amm) {
        // @TODO: test this
        dataPt *data = robot->read_curr();
    }

    return;
}

static void wlevel_thread(APES *robot) {
    assert(robot != NULL);

    while (!stop_wlevel) {
        // @TODO: test this
        dataPt *data = robot->read_wlevel();
    }

    return;
}

static void wob_thread(APES *robot) {
    assert(robot != NULL);

    while (!stop_wob) {
        // @TODO: test this
        dataPt *data = robot->read_wob();
    }

    return;
}


