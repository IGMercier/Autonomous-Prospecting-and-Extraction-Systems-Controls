#include <assert>
#include <unistd.h>
#include <error.h>
#include <pthread.h>
#include <wiringPiSPI.h>
#include "APES.h"
#include "libraries/pybind11/include/pybind11/embed.h"

namespace py = pybind11;
using namespace py::literals;

static void *wob_thread(void *arg);
static void *therm_thread(void *arg);
static void *amm_thread(void *arg);
static void *wlevel_thread(void *arg);

static pthread_mutex_t dataVector_lock;

APES::APES() {
    this->filename = NULL;
    this->file = NULL;
    this->wob = NULL;
    this->therm = NULL;
    this->amm = NULL;
    this->wlevel = NULL;
    this->motor = NULL;
}


/* object methods */
int APES::setup(char *filename) {

    if (filename != NULL) {
        this->filename = filename;
    } else {
        char *filename_default = "data.csv";
        this->filename = filename_default;
    }
    this->file = fopen(this->filename, "w");
    fprintf(this->file, "time, sensor, value\n");

    // @TODO: error handling
    pthread_mutex_init(&dataVector_lock, NULL);

    // @TODO: do i need this as a pointer??
    //this.dataArray = new std::vector<dataPt *>;
    
    // starts python interpreter
    py::initialize_interpreter();
    //@TODO: put the correct pin numbers
    this->wob = new Wob(2, 3);
    this->therm = new Therm(7, 27);
    this->amm = new Amm(5, 6);
    this->wlevel = new WLevel(2, 4);
    this->motor = new Motor(26, 8);
    
    wiringPiSPISetup(0, 500000);
    wiringPiSPISetup(1, 500000);
    return 0;
}

dataPt* APES::read_temp() {
    if (this->therm != NULL) {
        float temp = this->therm->read_temp();

        dataPt *data = new dataPt;
        data->time = std::chrono::system_clock::now();
        data->sensor = THERM_DATA;
        data->dataField.dataF = temp;

        return data;
    }
    return NULL;
}

dataPt* APES::D_temp() {
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

        return data;
    }
    return NULL;
}

dataPt* APES::read_wlevel() {
    if (this->wlevel != NULL) {
        float force = this->wlevel->read_wlevel();

        dataPt *data = new dataPt;
        data->time = std::chrono::system_clock::now();
        data->sensor = WLEVEL_DATA;
        data->dataField.dataI = force;

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

void APES::automatic() {
    pthread_t wob_tid;
    pthread_t therm_tid;
    pthread_t amm_tid;
    pthread_t wlevel_tid;

    // @TODO: error handling
    pthread_create(&wob_tid, NULL, wob_thread, (void *)(long)(this));
    pthread_create(&therm_tid, NULL, therm_thread, (void *)(long)(this));
    pthread_create(&amm_tid, NULL, amm_thread, (void *)(long)(this));
    pthread_create(&wlevel_tid, NULL, wlevel_thread, (void *)(long)(this));
}

void APES::standby() {
    // ensure that everything is off
    motor_stop();
}

void APES::finish() {
    // stop all actuators before deconstructor
    motor_stop();

    fclose(this->file);

    // kills the python interpreter
    // and deletes everything, so
    // you better actually want to kill
    // the system if you call this func
    writeDataVector();
    if (this->wob != NULL) { delete this->wob; }
    if (this->therm != NULL) { delete this->therm; }
    if (this->amm != NULL) { delete this->amm; }
    if (this->wlevel != NULL) { delete this->wlevel; }
    if (this->motor != NULL) { delete this->motor; }

    // super important that any pybind objects are
    // killed before pybind interpreter finalized!
    py::finalize_interpreter();

    pthread_mutex_destroy(&dataVector_lock);
    for (unsigned int  = 0; i < this->dataVector.size(); i++) {
        delete this->dataVector.at(i);
    }
    this->dataVector.clear();
}

void APES::readData(const char *filename) {
    return;
}

void APES::saveData(dataPt *data) {
    assert(data != NULL);
    if (this->dataVector.size() >= MAXDATA) {
        writeDataVector();
    } else {
        this->dataVector.push_back(data);
    }

}

void APES::writeDataVector() {
    for (unsigned int i = 0; i < this->dataVector.size(); i++) {
        dataPt *data = this->dataVector.at(i);

        switch(data->origin) {
            case THERM_DATA:
                fprintf(this->file, "%s, %li, %f\n",
                        "therm", data->time, data->dataField.dataF);
                break;
            case AMM_DATA:
                fprintf(this->file, "%s, %li, %f\n",
                        "amm", data->time, data->dataField.dataF);
                break;
            case WLEVEL_DATA:
                fprintf(this->file, "%s, %li, %d\n",
                        "level", data->time, data->dataField.dataI);
                break;
            case WOB_DATA:
                fprintf(this->file, "%s, %li, %f\n",
                        "wob", data->time, data->dataField.dataF);
                break;
            default:
                fprintf(this->file, "%s, %li, %f\n",
                        "none", data->time, data->dataField.dataF);
                break;
        }

        delete data;
    }

    this->dataVector.clear();
}

APES::~APES() {
}

static void *wob_thread(void *arg) {
}

static void *therm_thread(void *arg) {
    assert(arg != NULL);
    pthread_detach(pthread_self());

    APES robot = (APES)(long)arg;

    while (1) {
        // @TODO: test this
        float temp = robot->read_temp();

        pthread_mutex_lock(&dataVector_lock);
        robot->saveData();
        pthread_mutex_unlock(&dataVector_lock);
    }

    return NULL
}

static void *amm_thread(void *arg) {
}

static void *wlevel_thread(void *arg) {
}

