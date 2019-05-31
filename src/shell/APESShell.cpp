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

        if (cmdline == "quit") {
            toSend(shutdown_tag);
            break;
        }

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
            ctk->command = MOTOR_DRIVE;

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
            ctk->command = MOTOR_STOP;
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

        case MOTOR_DRIVE:
            {
                int dir = ctk.argv[0].dataI;
                int speed = ctk.argv[1].dataI;
                int time = ctk.argv[2].dataI;
                //this->robot->motor_drive(dir, speed, time);
                msg = "System's motor enabled for " + std::to_string(time) + " us!\n";
                shell->toSend(msg);
            }
            break;

        case MOTOR_STOP:
            //this->robot->motor_stop();
            msg = "System's motor disabled!\n";
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
    std::string msg = "Listing Help Commands!\n";
    shell->toSend(msg);

    msg = "start => setups APES system\n";
    shell->toSend(msg);

    msg = "standby => turns off actuators and stops auto mode\n";
    shell->toSend(msg);
    
    msg = "data => does something not specified\n";
    shell->toSend(msg);
    
    msg = "help => prints this help message\n";
    shell->toSend(msg);

    msg = "quit => quits APES, shell, and server systems\n";
    shell->toSend(msg);

    msg = "auto on & => automatically reads off sensor data\n";
    shell->toSend(msg);

    msg = "auto off => turns off auto mode\n";
    shell->toSend(msg);

    msg = "temp => reads off current temp\n";
    shell->toSend(msg);

    msg = "dtemp => reads off difference in temp since start\n";
    shell->toSend(msg);

    msg = "curr => reads off current current\n";
    shell->toSend(msg);

    msg = "wlevel => reads off current water level\n";
    shell->toSend(msg);

    msg = "wob => reads off current force\n";
    shell->toSend(msg);

    msg = "encoder => reads off current encoder data\n";
    shell->toSend(msg);

    msg = "motor drive <dir> <speed> <time> => runs motor\n";
    shell->toSend(msg);

    msg = "motor stop => stops motor\n";
    shell->toSend(msg);

    msg = "drill run => runs drill\n";
    shell->toSend(msg);

    msg = "drill stop => stops drill\n";
    shell->toSend(msg);

    msg = "drill cycle <dc> => changes drill duty cycle\n";
    shell->toSend(msg);

    return;
}
