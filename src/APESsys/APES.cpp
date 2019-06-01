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

    this->wob = NULL;
    this->therm = NULL;
    this->amm = NULL;
    this->wlevel = NULL;
    this->motor_X = NULL;
    this->motor_Y = NULL;


    // starts python interpreter
    py::initialize_interpreter();
}


/* object methods */
int APES::setup() {
    //@TODO: put the correct pin numbers 
    this->wob = new Wob(WOB_DATA_PIN, WOB_CLOCK_PIN);
    this->therm = new Therm(THERM_BUS, THERM_CHAN);
    this->amm = new Amm(AMM_BUS, AMM_CHAN);
    this->wlevel = new WLevel(WLEVEL_BUS, WLEVEL_CHAN_START, WLEVEL_CHAN_END);
    this->motor_Y = new Motor(PUMP_DIR_PIN, PUMP_SPEED_PIN);
    this->motor_X = new Motor(26, 8);

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
        int ticks = this->encoder->getTicks();

        dataPt *data = new dataPt;
        data->time = std::chrono::system_clock::now();
        data->sensor = ENCODER_DATA;
        data->dataField.dataI = ticks;
        
        std::unique_lock<std::mutex> datalock(*(this->data_mtx));
        saveData(data);
        datalock.unlock();

        return data;
    }
    return NULL;
}

void APES::motor_X_drive(bool dir, int speed, int time) {
    if (this->motor_X != NULL) {
        this->motor_X->motor_drive(dir, speed, time);
    }
}

void APES::motor_Y_drive(bool dir, int speed, int time) {
    if (this->motor_Y != NULL) {
        this->motor_Y->motor_drive(dir, speed, time);
    }
}

void APES::motor_X_stop() {
    if (this->motor_X != NULL) {
        this->motor_X->motor_stop();
    }
}

void APES::motor_Y_stop() {
    if (this->motor_Y != NULL) {
        this->motor_Y->motor_stop();
    }
}

void APES::auto_on(autoFunc which) {

    while (1) {
        if (which & AUTO_THERM) {
            if (!stop_therm) {
                read_temp();
            }
        }
        
        if (which & AUTO_AMM) {
            if !(stop_amm) {
                read_amm();
            }
        }

        if (which & AUTO_WLEVEL) {
            if (!stop_wlevel) {
                read_wlevel();
            }
        }

        if (which & AUTO_WOB) {
            if (!stop_wob) {
                read_wob();
            }
        }
        
        if (which & AUTO_ENCODER) {
            if (!stop_encoder) {
                read_encoder();
            }
        }

        if (stop_therm && stop_amm &&
            stop_wlevel && stop_wob &&
            stop_encoder) {
            break;
        }

        std::this_thread::sleep_for(
            std::chrono::milliseconds(SLEEP_INTVL)
        );
    }

    return;
}

void APES::auto_off(autoFunc which) {
    if (which & AUTO_THERM) {
        stop_therm = 1;
    }
    if (which & AUTO_AMM) {
        stop_amm = 1;
    }
    if (which & AUTO_WLEVEL) {
        stop_wlevel = 1;
    }
    if (which & AUTO_WOB) {
        stop_wob = 1;
    }
    return;
}

void APES::standby() {
    // ensure that everything is off
    motor_X_stop();
    motor_Y_stop();
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

    motor_Y_stop();
    motor_X_stop();

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
