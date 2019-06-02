#include <cstdlib>
#include <cstring>
#include <thread>
#include <string>
#include <unistd.h>
#include <assert.h>

#include "APESShell.h"
//#include "../APESsys/APES.h"
#include "../APESsys/commands.h"

using std::thread;
static void execute(parse_token *ltk, APESShell *shell);
static void listCommands(APESShell *shell);

APESShell::APESShell(sysArgs *args) {
    assert(args != NULL);
    assert(args->cmd_mtx != NULL);
    assert(args->log_mtx != NULL);
    assert(args->cmdq != NULL);
    assert(args->logq != NULL);

    //this->robot = new APES();
    this->cmd_mtx = args->cmd_mtx;
    this->log_mtx = args->log_mtx;
    this->cmdq = args->cmdq;
    this->logq = args->logq;
}

void APESShell::run() {
    while (1) {
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
        if (cmdline == shutdown_tag)
            break;

        evaluate(cmdline);
    }
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
        thread temp(execute, &tk, this);
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

    } else if (ltk->argv[0] == "motor") {

        if (ltk->argv[1] == "drive") {
            ctk->command = MOTOR_Z_DRIVE;

            try {
                rc = std::stoi(ltk->argv[2]); // dir
            } catch (...) {
                ctk->command = NONE;
                return;
            }
            ctk->argv[(ctk->argc)++].dataI = rc;

            try {
                rc = std::stoi(ltk->argv[3]); // speed
            } catch (...) {
                ctk->command = NONE;
                return;
            }
            ctk->argv[(ctk->argc)++].dataI = rc;

            try {
                rc = std::stoi(ltk->argv[4]); // time  
            } catch (...) {
                ctk->command = NONE;
                return;
            }
            ctk->argv[(ctk->argc)++].dataI = rc;
            return;

        } else if (ltk->argv[1] == "stop") {
            ctk->command = MOTOR_Z_STOP;
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
                rc = std::stoi(ltk->argv[3]); // speed
            } catch (...) {
                ctk->command = NONE;
                return;
            }
            ctk->argv[(ctk->argc)++].dataI = rc;

            try {
                rc = std::stoi(ltk->argv[4]); // time  
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

static void execute(parse_token *ltk, APESShell *shell) {
    assert(ltk != NULL);
    assert(shell != NULL);

    command_token ctk;

    shell->parsecommand(ltk, &ctk);
    
    command_state command = ctk.command;
    std::string msg;
    if (ltk->bg) {
        msg = "BG job: ";
        shell->toSend(msg);
    }

    switch (command) {
        case START:
            //this->robot->setup();
            //this->robot->standby();
            msg = "System started!\n";
            shell->toSend(msg);
            break;

        case STANDBY:
            //this->robot->standby();
            msg = "System in standby!\n";
            shell->toSend(msg);
            break;

        case DATA:
            //this->robot->readData();
            msg = "Reading from data file!\n";
            shell->toSend(msg);
            break;

        case HELP:
	        listCommands(shell);
            break;

        case QUIT:
            //this->robot->finish();
            msg = "System shutting down!\n";
            shell->toSend(msg);
            shell->toSend(shutdown_tag);
            break;

        case AUTO_ON:
            {
                msg = "System's auto mode enabled!\n";
                shell->toSend(msg);
                //this->robot->auto_on();
                //std::thread sensort(shell->robot->auto_on, /*fill this with param*/);
                //if (sensort.joinable()) {
                //    sensort.join();
                //}
            }
            break;

        case AUTO_OFF:
            //this->robot->auto_off();
            msg = "System's auto mode disabled!\n";
            shell->toSend(msg);
            break;

        case SOL_0_OPEN:
            //this->robot->sol_0_open();
            msg = "System's solenoid 0 opened!\n";
            shell->toSend(msg);
            break;

        case SOL_0_CLOSE:
            //this->robot->sol_0_close();
            msg = "System's solenoid 0 closed!\n";
            shell->toSend(msg);
            break;
        
        case SOL_1_OPEN:
            //this->robot->sol_1_open();
            msg = "System's solenoid 1 opened!\n";
            shell->toSend(msg);
            break;

        case SOL_1_CLOSE:
            //this->robot->sol_1_close();
            msg = "System's solenoid 1 closed!\n";
            shell->toSend(msg);
            break;
        
        case HEATER_0_ON:
            //this->robot->heater_0_on();
            msg = "System's dc heater 0 on!\n";
            shell->toSend(msg);
            break;
        
        case HEATER_0_OFF:
            //this->robot->heater_0_off();
            msg = "System's dc heater 0 off!\n";
            shell->toSend(msg);
            break;
        
        case HEATER_1_ON:
            //this->robot->heater_1_on();
            msg = "System's dc heater 1 on!\n";
            shell->toSend(msg);
            break;
        
        case HEATER_1_OFF:
            //this->robot->heater_1_off();
            msg = "System's dc heater 1 off!\n";
            shell->toSend(msg);
            break;

        case RELAY_0_ON:
            //this->robot->relay_0_on();
            msg = "System's relay 0 on!\n";
            shell->toSend(msg);
            break;
        
        case RELAY_0_OFF:
            //this->robot->relay_0_off();
            msg = "System's relay 0 off!\n";
            shell->toSend(msg);
            break;
        
        case RELAY_1_ON:
            //this->robot->relay_1_on();
            msg = "System's relay 1 on!\n";
            shell->toSend(msg);
            break;
        
        case RELAY_1_OFF:
            //this->robot->relay_1_off();
            msg = "System's relay 1 off!\n";
            shell->toSend(msg);
            break;
            

        case TEMP:
            //this->robot->read_temp();
            msg = "Reading temp!\n";
            shell->toSend(msg);
            break;

        case DTEMP:
            //this->robot->read_dtemp();
            msg = "Reading dtemp!\n";
            shell->toSend(msg);
            break;

        case CURR:
            //this->robot->read_curr();
            msg = "Reading curr!\n";
            shell->toSend(msg);
            break;

        case WLEVEL:
            //this->robot->read_wlevel();
            msg = "Reading wlevel!\n";
            shell->toSend(msg);
            break;

        case WOB:
            //this->robot->read_wob();
            msg = "Reading wob!\n";
            shell->toSend(msg);
            break;

        case MOTOR_Z_DRIVE:
            {
                int dir = ctk.argv[0].dataI;
                int speed = ctk.argv[1].dataI;
                int time = ctk.argv[2].dataI;
                //this->robot->motor_Z_drive(dir, speed, time);
                msg = "System's Z-axis motor enabled for " + std::to_string(time) + " us!\n";
                shell->toSend(msg);
            }
            break;

        case MOTOR_Z_STOP:
            //this->robot->motor_Z_stop();
            msg = "System's Z-axis motor disabled!\n";
            shell->toSend(msg);
            break;
        
        case PUMP_DRIVE:
            {
                int dir = ctk.argv[0].dataI;
                int speed = ctk.argv[1].dataI;
                int time = ctk.argv[2].dataI;
                //this->robot->pump_drive(dir, speed, time);
                msg = "System's pump enabled for " + std::to_string(time) + " us!\n";
                shell->toSend(msg);
            }
            break;

        case PUMP_STOP:
            //this->robot->pump_stop();
            msg = "System's pump disabled!\n";
            shell->toSend(msg);
            break;

        case DRILL_RUN:
            msg = "System's drill enabled!\n";
            shell->toSend(msg);
            break;

        case DRILL_STOP:
            msg = "System's drill disabled!\n";
            shell->toSend(msg);
            break;

        case DRILL_CYCLE:
            {
                int dc = ctk.argv[0].dataI;
                int on_period = ctk.argv[1].dataI;
                float freq = ctk.argv[2].dataF;
                //shell->robot->drill_cycle(dc, on_period, freq);
                msg = "System's drill duty cycle changed!\n";
                shell->toSend(msg);
            }
            break;

        case NONE:
        default:
            msg = "Not a valid command (use 'help' for more info)!\n";
            shell->toSend(msg);
            break;
    }
}

APESShell::~APESShell() {
    //this->robot->finish();
    //delete this->robot;
}

static void listCommands(APESShell *shell) {
    std::string msg = "Listing Help Commands!";
    shell->toSend(msg);

    msg = "start => setups APES system";
    shell->toSend(msg);

    msg = "standby => turns off actuators and stops auto mode";
    shell->toSend(msg);
    
    msg = "data => does something not specified";
    shell->toSend(msg);
    
    msg = "help => prints this help message";
    shell->toSend(msg);

    msg = "quit => quits APES, shell, and server systems";
    shell->toSend(msg);

    msg = "auto on & => automatically reads off sensor data";
    shell->toSend(msg);

    msg = "auto off => turns off auto mode";
    shell->toSend(msg);

    msg = "temp => reads off current temp";
    shell->toSend(msg);

    msg = "dtemp => reads off difference in temp since start";
    shell->toSend(msg);

    msg = "curr => reads off current current";
    shell->toSend(msg);

    msg = "wlevel => reads off current water level";
    shell->toSend(msg);

    msg = "wob => reads off current force";
    shell->toSend(msg);

    msg = "encoder => reads off current encoder data";
    shell->toSend(msg);

    msg = "motor drive <dir> <speed> <time> => runs motor";
    shell->toSend(msg);

    msg = "motor stop => stops motor";
    shell->toSend(msg);

    msg = "drill run => runs drill";
    shell->toSend(msg);

    msg = "drill stop => stops drill";
    shell->toSend(msg);

    msg = "drill cycle <dc> => changes drill duty cycle";
    shell->toSend(msg);

    return;
}
