#include <cstdlib>
#include <cstring>
#include <thread>
#include <chrono>
#include <atomic>
#include <string>
#include <unistd.h>
#include <assert.h>

#include "APESShell.h"

using std::thread;

std::atomic_int sensor_auto = {0};
std::atomic_int shutdown_sig = {0};
        
constexpr const std::chrono::milliseconds therm_period(50);
constexpr const std::chrono::milliseconds amm_period(50);
constexpr const std::chrono::milliseconds wob_period(50);
constexpr const std::chrono::milliseconds enc_period(50);

static void sigsegv_handler(int sig);

APESShell::APESShell(sysArgs *args) {
    assert(args != nullptr);
    assert(args->cmd_mtx != nullptr);
    assert(args->log_mtx != nullptr);
    assert(args->data_mtx != nullptr);
    assert(args->cmdq != nullptr);
    assert(args->logq != nullptr);
    signal(SIGSEGV, sigsegv_handler);

    this->robot = new APES("data.csv", args->data_mtx);
    this->cmd_mtx = args->cmd_mtx;
    this->log_mtx = args->log_mtx;
    this->cmdq = args->cmdq;
    this->logq = args->logq;
}

void APESShell::run() {
    while (!shutdown_sig.load()) {
        std::string cmdline;
        std::unique_lock<std::mutex> cmdlock(*(this->cmd_mtx));
        if (!this->cmdq->empty()) {
            cmdline = this->cmdq->at(0);
            this->cmdq->pop_front();
        } else {
            cmdlock.unlock();
            continue;
        }
        cmdlock.unlock();
        if (cmdline == shutdown_tag) {
            shutdown_sig.store(1);
            break;
        }

        evaluate(cmdline);
    }
    shutdown();
    return; // kills shell thread in main program
}

void APESShell::evaluate(std::string cmdline) {
    int bg;
    parse_token tk;
    thread child;

    bg = parseline(cmdline, &tk);

    if (tk.argc == 0) { return; }

    if (!builtin_command(&tk)) {
        /* CHILD THREAD */
        thread temp(&APESShell::execute, this, &tk);
        child.swap(temp);

        if (bg) { child.detach(); }
    }

    /* PARENT THREAD */
    if (!bg) {
        if (child.joinable()) {
            child.join();
        }
    }
    return;
}

void APESShell::toSend(std::string msg) {
    std::unique_lock<std::mutex> loglock(*(this->log_mtx));
    this->logq->push_back(msg);
    loglock.unlock();
}

void APESShell::parsecommand(parse_token *ltk, command_token *ctk) {
    int rc;
    ctk->argc = 0;

    if (ltk->argv[0] == "start") {
        ctk->command = START;
        return;

    } else if (ltk->argv[0] == "standby") {
        ctk->command = STANDBY;
        return;

    } else if (ltk->argv[0] == "data") {
        ctk->command = DATA;
        return;

    } else if (ltk->argv[0] == "help") {
        ctk->command = HELP;
        return;

    } else if (ltk->argv[0] == "quit") {
        ctk->command = QUIT;
        return;

    } else if (ltk->argv[0] == "sol") {
        if (ltk->argv[1] == "0") {
            if (ltk->argv[2] == "open") {
                ctk->command = SOL_0_OPEN;
                return;
            } else if (ltk->argv[2] == "close") {
                ctk->command = SOL_0_CLOSE;
                return;
            }

        } else if (ltk->argv[1] == "1") {
            if (ltk->argv[2] == "open") {
                ctk->command = SOL_1_OPEN;
                return;
            } else if (ltk->argv[2] == "close") {
                ctk->command = SOL_1_CLOSE;
                return;
            }

        }
    } else if (ltk->argv[0] == "heater") {
        if (ltk->argv[1] == "0") {
            if (ltk->argv[2] == "on") {
                ctk->command = HEATER_0_ON;
                return;
            } else if (ltk->argv[2] == "off") {
                ctk->command = HEATER_0_OFF;
                return;
            }

        } else if (ltk->argv[1] == "1") {
            if (ltk->argv[2] == "on") {
                ctk->command = HEATER_1_ON;
                return;
            } else if (ltk->argv[2] == "off") {
                ctk->command = HEATER_1_OFF;
                return;
            }

        }
    } else if (ltk->argv[0] == "relay") {
        if (ltk->argv[1] == "0") {
            if (ltk->argv[2] == "on") {
                ctk->command = RELAY_0_ON;
                return;
            } else if (ltk->argv[2] == "off") {
                ctk->command = RELAY_0_OFF;
                return;
            }

        } else if (ltk->argv[1] == "1") {
            if (ltk->argv[2] == "on") {
                ctk->command = RELAY_1_ON;
                return;
            } else if (ltk->argv[2] == "off") {
                ctk->command = RELAY_1_OFF;
                return;
            }

        }
    } else if (ltk->argv[0] == "auto") {
        if (!ltk->bg) {
            ctk->command = NONE;
            return;
        }
        if (ltk->argv[1] == "on") {
            ctk->command = AUTO_ON;
            return;
        } else if (ltk->argv[1] == "off") {
            ctk->command = AUTO_OFF;
            return;
        }

    } else if (ltk->argv[0] == "temp") {
        ctk->command = TEMP;
        return;

    } else if (ltk->argv[0] == "dtemp") {
        ctk->command = DTEMP;
        return;

    } else if (ltk->argv[0] == "curr") {
        ctk->command = CURR;
        return;

    } else if (ltk->argv[0] == "wlevel") {
        ctk->command = WLEVEL;
        return;

    } else if (ltk->argv[0] == "wob") {
        ctk->command = WOB;
        return;

    } else if (ltk->argv[0] == "encoder") {
        if (ltk->argv[1] == "reset") {
            ctk->command = ENCODER_RESET;
            return;
        } else {
            ctk->command = ENCODER;
            return;
        }

    } else if (ltk->argv[0] == "stepper") {

        if (ltk->argv[1] == "drive") {
            ctk->command = STEPPER_DRIVE;

            try {
                rc = std::stoi(ltk->argv[2]); // steps
            } catch (...) {
                ctk->command = NONE;
                return;
            }
            ctk->argv[(ctk->argc)++].dataI = rc;

            try {
                rc = std::stof(ltk->argv[3]); // freq
            } catch (...) {
                ctk->command = NONE;
                return;
            }
            ctk->argv[(ctk->argc)++].dataF = rc;
            return;

        } else if (ltk->argv[1] == "stop") {
            ctk->command = STEPPER_STOP;
            return;

        }

    } else if (ltk->argv[0] == "pump") {

        if (ltk->argv[1] == "drive") {
            ctk->command = PUMP_DRIVE;

            try {
                rc = std::stoi(ltk->argv[2]); // dir
            } catch (...) {
                ctk->command = NONE;
                return;
            }
            ctk->argv[(ctk->argc)++].dataI = rc;

            try {
                rc = std::stoi(ltk->argv[3]); // dc
            } catch (...) {
                ctk->command = NONE;
                return;
            }
            ctk->argv[(ctk->argc)++].dataI = rc;

            return;

        } else if (ltk->argv[1] == "stop") {
            ctk->command = PUMP_STOP;
            return;

        }

    } else if (ltk->argv[0] == "drill") {
        if (ltk->argv[1] == "run") {
            ctk->command = DRILL_RUN;
            return;

        } else if (ltk->argv[1] == "stop") {
            ctk->command = DRILL_STOP;
            return;

        } else if (ltk->argv[1] == "cycle") {
            ctk->command = DRILL_CYCLE;

            try {
                rc = std::stoi(ltk->argv[2]); // duty cycle
            } catch (...) {
                ctk->command = NONE;
                return;
            }
            ctk->argv[(ctk->argc)++].dataI = rc;

            try {
                rc = std::stoi(ltk->argv[3]); // on_period
            } catch (...) {
                ctk->command = NONE;
                return;
            }
            ctk->argv[(ctk->argc)++].dataI = rc;

            float rcf;
            try {
                rcf = std::stof(ltk->argv[4]); // freq
            } catch (...) {
                ctk->command = NONE;
                return;
            }
            ctk->argv[(ctk->argc)++].dataF = rcf;
            return;
        }
    }

    ctk->command = NONE;

    return;
}

void APESShell::execute(parse_token *ltk) {
    assert(ltk != nullptr);

    command_token ctk;

    parsecommand(ltk, &ctk);
    
    command_state command = ctk.command;
    std::string msg;
    if (ltk->bg) {
        msg = "BG job: ";
        toSend(msg);
    }

    switch (command) {
        case START:
            {
                auto_off();
                this->robot->setup();
                this->robot->standby();
                msg = "System started!\n";
                toSend(msg);
            }
            break;

        case STANDBY:
            {
                auto_off();
                this->robot->standby();
                msg = "System in standby!\n";
                toSend(msg);
            }
            break;

        case DATA:
            {
                toSend(data_tag);
            }
            break;

        case HELP:
            {
                listCommands();
            }
            break;

        case QUIT:
            {
                shutdown_sig.store(1);
                shutdown();
                msg = "System shutting down!\n";
                toSend(msg);
                toSend(shutdown_tag);
            }
            break;

        case AUTO_ON:
            {
                msg = "System's auto mode enabled!\n";
                toSend(msg);
                auto_on();
            }
            break;

        case AUTO_OFF:
            {
                auto_off();
                msg = "System's auto mode disabled!\n";
                toSend(msg);
            }
            break;

        case SOL_0_OPEN:
            {
                this->robot->sol_0_open();
                msg = "System's solenoid 0 opened!\n";
                toSend(msg);
            }
            break;

        case SOL_0_CLOSE:
            {
                this->robot->sol_0_close();
                msg = "System's solenoid 0 closed!\n";
                toSend(msg);
            }
            break;
        
        case SOL_1_OPEN:
            {
                this->robot->sol_1_open();
                msg = "System's solenoid 1 opened!\n";
                toSend(msg);
            }
            break;

        case SOL_1_CLOSE:
            {
                this->robot->sol_1_close();
                msg = "System's solenoid 1 closed!\n";
                toSend(msg);
            }
            break;
        
        case HEATER_0_ON:
            {
                this->robot->heater_0_on();
                msg = "System's dc heater 0 on!\n";
                toSend(msg);
            }
            break;
        
        case HEATER_0_OFF:
            {
                this->robot->heater_0_off();
                msg = "System's dc heater 0 off!\n";
                toSend(msg);
            }
            break;
        
        case HEATER_1_ON:
            {
                this->robot->heater_1_on();
                msg = "System's dc heater 1 on!\n";
                toSend(msg);
            }
            break;
        
        case HEATER_1_OFF:
            {
                this->robot->heater_1_off();
                msg = "System's dc heater 1 off!\n";
                toSend(msg);
            }
            break;

        case RELAY_0_ON:
            {
                this->robot->relay_0_on();
                msg = "System's relay 0 on!\n";
                toSend(msg);
            }
            break;
        
        case RELAY_0_OFF:
            {
                this->robot->relay_0_off();
                msg = "System's relay 0 off!\n";
                toSend(msg);
            }
            break;
        
        case RELAY_1_ON:
            {
                this->robot->relay_1_on();
                msg = "System's relay 1 on!\n";
                toSend(msg);
            }
            break;
        
        case RELAY_1_OFF:
            {
                this->robot->relay_1_off();
                msg = "System's relay 1 off!\n";
                toSend(msg);
            }
            break;

        case TEMP:
            {
                dataPt *data = this->robot->read_temp();
                float temp = data->dataField.dataF;
                long long int time = data->time;
		        msg = "<data>" + std::to_string(time) + ", TEMP, " + std::to_string(temp) + "</data>";
                toSend(msg);
            }
            break;

        case DTEMP:
            {
                dataPt *data = this->robot->read_dtemp();
                float dtemp = data->dataField.dataF;
                long long int time = data->time;
                msg = "<data>" + std::to_string(time) + ", DTEMP, " + std::to_string(dtemp) + "</data>";
                toSend(msg);
            }
            break;

        case CURR:
            {
                dataPt *data = this->robot->read_curr();
                float curr = data->dataField.dataF;
                long long int time = data->time;
                msg = "<data>" + std::to_string(time) + ", CURR, " + std::to_string(curr) + "</data>";
                toSend(msg);
            }
            break;

        case WLEVEL:
            {
                dataPt *data = this->robot->read_wlevel();
                int wlevel = data->dataField.dataI;
                long long int time = data->time;
                msg = "<data>" + std::to_string(time) + ", WLEVEL, " + std::to_string(wlevel) + "</data>";
                toSend(msg);
            }
            break;

        case WOB:
            {
                dataPt *data = this->robot->read_wob();
                float force = data->dataField.dataF;
                long long int time = data->time;
                msg = "<data>" + std::to_string(time) + ", WOB, " + std::to_string(force) + "</data>";
                toSend(msg);
            }
            break;

        case ENCODER:
            {
                dataPt *data = this->robot->read_encoder();
                unsigned int pulse = data->dataField.dataUI;
                long long int time = data->time;
                msg = "<data>" + std::to_string(time) + ", ENCODER, " + std::to_string(pulse) + "</data>";
                toSend(msg);
            }
            break;

        case ENCODER_RESET:
            {
                this->robot->reset_encoder();
                msg = "Encoder reset!\n";
                toSend(msg);
            }
            break;

        case STEPPER_DRIVE:
            {
                int steps = ctk.argv[0].dataI;
                float freq = ctk.argv[1].dataF;
                msg = "System's stepper motor enabled for " +
                      std::to_string(steps) + " steps at " +
                      std::to_string(freq) + "Hz!\n";
                toSend(msg);
                this->robot->stepper_drive(steps, freq);
            }
            break;

        case STEPPER_STOP:
            {
                this->robot->stepper_stop();
                msg = "System's stepper motor disabled!\n";
                toSend(msg);
            }
            break;
        
        case PUMP_DRIVE:
            {
                int dir = ctk.argv[0].dataI;
                int dc = ctk.argv[1].dataI;
                this->robot->pump_drive(dir, dc);
                msg = "System's pump enabled!\n";
                toSend(msg);
            }
            break;

        case PUMP_STOP:
            {
                this->robot->pump_stop();
                msg = "System's pump disabled!\n";
                toSend(msg);
            }
            break;

        case DRILL_RUN:
            {
                int dc = ctk.argv[0].dataI;
                float freq = ctk.argv[1].dataF;
                this->robot->drill_run(dc, freq);
                msg = "System's drill enabled!\n";
                toSend(msg);
            }
            break;

        case DRILL_STOP:
            {
                this->robot->drill_stop();
                msg = "System's drill disabled!\n";
                toSend(msg);
            }
            break;

        case DRILL_CYCLE:
            {
                int dc = ctk.argv[0].dataI;
                int on_period = ctk.argv[1].dataI;
                float freq = ctk.argv[2].dataF;
                //this->robot->drill_cycle(dc, on_period, freq);
                msg = "System's drill duty cycle changed!\n";
                toSend(msg);
            }
            break;

        case NONE:
        default:
            {
                msg = "Not a valid command (use 'help' for more info)!\n";
                toSend(msg);
            }
            break;
    }
}

void APESShell::auto_on() {
    std::string msg;
    if (sensor_auto.load()) {
        msg = "Sensors already running!";
        toSend(msg);
        return;
    }

    sensor_auto.store(1);
    msg = "Initiated automatic sensing!";
    toSend(msg);
    std::chrono::time_point<std::chrono::high_resolution_clock> now, therm_next, amm_next, wob_next, enc_next;

    therm_next = std::chrono::high_resolution_clock::now();
    amm_next = std::chrono::high_resolution_clock::now();
    wob_next = std::chrono::high_resolution_clock::now();
    enc_next = std::chrono::high_resolution_clock::now();
    while (sensor_auto.load()) {
        now = std::chrono::high_resolution_clock::now();
        
        if (now > therm_next) { 
            therm_next = now + therm_period;
            dataPt *data = this->robot->read_temp();
            float temp = data->dataField.dataF;
            long long int time = data->time;
		    msg = "<data>" + std::to_string(time) + ", TEMP, " + std::to_string(temp) + "</data>";
            toSend(msg);
        }
        
        if (now > amm_next) {
            amm_next = now + amm_period;
            dataPt *data = this->robot->read_curr();
            float curr = data->dataField.dataF;
            long long int time = data->time;
            msg = "<data>" + std::to_string(time) + ", CURR, " + std::to_string(curr) + "</data>";
            toSend(msg);
        }

        if (now > wob_next) {
            wob_next = now + wob_period;
            dataPt *data = this->robot->read_wob();
            float force = data->dataField.dataF;
            long long int time = data->time;
            msg = "<data>" + std::to_string(time) + ", WOB, " + std::to_string(force) + "</data>";
            toSend(msg);
        }

        if (now > enc_next) {
            enc_next = now + enc_period;
            dataPt *data = this->robot->read_encoder();
            unsigned int pulse = data->dataField.dataUI;
            long long int time = data->time;
            msg = "<data>" + std::to_string(time) + ", ENCODER, " + std::to_string(pulse) + "</data>";
            toSend(msg);
        }
    }
}

void APESShell::auto_off() {
    sensor_auto.store(0);
    std::string msg;
    msg = "Stopping automatic sensing!";
    toSend(msg);
    return;
}

APESShell::~APESShell() {
    if (this->robot != nullptr) {
        this->robot->standby();
        this->robot->finish();
        delete this->robot;
    }
}

void APESShell::shutdown() {
    auto_off();
    if (this->robot != nullptr) {
        this->robot->standby();
        this->robot->finish();
        delete this->robot;
    }
}

void APESShell::listCommands() {
    std::string msg = "Listing Help Commands!\n";
    toSend(msg);

    msg = "start => setups APES system\n";
    toSend(msg);

    msg = "standby => turns off actuators and stops auto mode\n";
    toSend(msg);
    
    msg = "data => does something not specified\n";
    toSend(msg);
    
    msg = "help => prints this help message\n";
    toSend(msg);

    msg = "quit => quits APES, shell, and server systems\n";
    toSend(msg);

    msg = "auto on & => automatically reads off sensor data\n";
    toSend(msg);

    msg = "auto off => turns off auto mode\n";
    toSend(msg);

    msg = "sol 0 <open/close> => opens and closes the solenoid valve\n";
    toSend(msg);

    msg = "heater 0 <on/off> => enables or disables the drill heater\n";
    toSend(msg);

    msg = "relay 0 <on/off> => enables or disables the water heater\n";
    toSend(msg);

    msg = "auto <on/off> & => enables or disables automatic sensor data (MUST be background job!)\n";
    toSend(msg);

    msg = "temp => reads off current temp\n";
    toSend(msg);

    msg = "dtemp => reads off difference in temp since start\n";
    toSend(msg);

    msg = "curr => reads off current current\n";
    toSend(msg);

    msg = "wlevel => reads off current water level\n";
    toSend(msg);

    msg = "wob => reads off current force\n";
    toSend(msg);

    msg = "encoder => reads off current encoder data\n";
    toSend(msg);

    msg = "motor drive <dir> <speed> <time> => runs motor\n";
    toSend(msg);

    msg = "motor stop => stops motor\n";
    toSend(msg);

    msg = "drill run => runs drill\n";
    toSend(msg);

    msg = "drill stop => stops drill\n";
    toSend(msg);

    msg = "drill cycle <dc> => changes drill duty cycle\n";
    toSend(msg);

    return;
}

static void sigsegv_handler() {
    shutdown_sig.store(1);
}
