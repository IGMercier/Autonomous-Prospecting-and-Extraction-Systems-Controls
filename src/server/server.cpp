#include "server.h"
#include "commands.h"
//#include "../APES.h"

//static APES robot;
static volatile int disconnected = 1;
static volatile int shutdownSIG = 0;

static void sigint_handler(int sig);
static void sigpipe_handler(int sig);

Server::Server() {
    signal(SIGINT, signint_handler);
    signal(SIGPIPE, sigpipe_handler);
    setCommands();
    //robot = APES();
}

void* Server::thread(void *arg) {
    pthread_detach(pthread_self());

    disconnected = 0;

    while (!disconnected) {
        if (shutdownSIG) { shutdown(); }

        char *cmdline = (char *)calloc(MAXLINE, sizeof(char));
        token tk;

        readFromClient(cmdline);
        parseline(cmdline, &tk);
        command(&tk);

        free(cmdline);
    }
    //robot.standby();
    close(this->cfd);
    return NULL;

}
int Server::command(token *tk) {
    assert(tk != NULL);

    command_state command = tk->command;
    std::string msg;

    switch (command) {
        case START:
            // setup robot and retry on fail
            /*while (robot.setup("data.csv") < 0) {
                fprintf(stderr, "ERROR: APES system setup failure!\n");
                fprintf(stdout, "Retrying...\n");
                robot.finish();
            }
            */

            // put in standby
            //robot.standby();
            msg = "System started!\n";
            sendToClient(msg.c_str());
            return 1;
        case HELP:
            listCommands();
            return 1;
        case QUIT:
            // shuts down everything
            shutdown();
            return 1;
        case AUTO:
            // runs things automatically
            /*
                @TODO:
                Will this need to create a new thread?
                We want this to run in the background,
                but we also want to be able to input more
                commands...
                If we spawn a process, APES system will be copied,
                so we dont want that...
            */
            msg = "System in auto mode!\n";
            sendToClient(msg.c_str());
            return 1;
        case TEMP:
            /*
            float temp;
            temp = robot.read_temp();
            printf(stdout, "Temp (@time): %f\n", temp);
            */
            msg = "Temp (@time): \n";
            sendToClient(msg.c_str());
            return 1;
        case DTEMP:
            /*
            float dtemp;
            dtemp = robot.D_temp();
            */
            msg = "Temp since init: \n";
            sendToClient(msg.c_str());
            return 1;
        case CURR:
            /*
            float curr;
            curr = robot.read_curr();
            printf(stdout, "Curr (@time): %f\n", curr);
            */
            msg = "Curr (@time): \n";
            sendToClient(msg.c_str());
            return 1;
        case WLEVEL:
            /*
            int level;
            level = robot.read_level();
            printf(stdout, "Level (@time): %d\n", level);
            */
            msg = "Level (@time): \n";
            sendToClient(msg.c_str());
            return 1;
        case STANDBY:
            //robot.standby();
            msg = "System in standby!\n";
            sendToClient(msg.c_str());
            return 1;
        case WOB:
            /*
            float force;
            force = robot.read_wob();
            printf(stdout, "Force (@time): %f\n", force);
            */
            msg = "Force (@time): \n";
            sendToClient(msg.c_str());
            return 1;
        case DATA:
            //robot.read_data();
            msg = "Server: Reading from data file...\n";
            sendToClient(msg.c_str());
            return 1;
        case MOTOR_DRIVE:
            fprintf(stdout, "COMMAND IS MOTOR_DRIVE!\n");
            //robot.motor_drive();
            return 1;
        case MOTOR_STOP:
            fprintf(stdout, "COMMAND IS MOTOR_STOP!\n");
            //robot.motor_stop();
            return 1;
        // do things for switch
        case DRILL_RUN:
            // runs drill
            fprintf(stdout, "COMMAND IS DRILL_RUN!\n");
            return 1;
        case DRILL_STOP:
            // stops drill
            fprintf(stdout, "COMMAND IS DRILL_STOP!\n");
            return 1;
        case DRILL_CYCLE:
            // runs drill at duty cycle
            fprintf(stdout, "COMMAND IS DRILL_CYCLE!\n");
            return 1;
        case NONE:
        default:
            //string text = tk->text;
            msg = "Error: Unknown command!\n";// + text;
            sendToClient(msg.c_str());
            // not a built-in command
            return 0;
    }
}

void Server::setCommands() {
    this->commandList.push_back("Help -- Commands:\n");
    this->commandList.push_back("help := prints this message\n");
    this->commandList.push_back("start := initializes system\n");
    this->commandList.push_back("standby := puts system in standby\n");
    this->commandList.push_back("temp := returns current temperature\n");
    this->commandList.push_back("dtemp := returns temperature differnce from initialization\n");
    this->commandList.push_back("curr := returns current amperage\n");
    this->commandList.push_back("wlevel := returns current water level\n");
    this->commandList.push_back("data := reads out data file\n");
    this->commandList.push_back("motor_drive := runs the motor\n");
    this->commandList.push_back("motor_stop := stops the motor\n");
    this->commandList.push_back("drill_run := runs the drill\n");
    this->commandList.push_back("drill_stop := stops the drill\n");
    this->commandList.push_back("drill_cycle := changes drill duty cycle\n");
    this->commandList.push_back("auto := sensors read automatically\n");
    this->commandList.push_back("quit := shuts down system, including server\n");
    return;
}

void Server::listCommands() {
    for (unsigned int i = 0; i < this->commandList.size(); i++) {
        sendToClient((this->commandList.at(i)).c_str());
    }
    return;
}

void Server::shutdown() {
    std::string msg = "System shutting down!\n";
    sendToClient(msg.c_str());
    //robot.finish();

    std::string msg = "Server shutting down!\n";
    sendToClient(msg.c_str());
    fprintf(stdout, "%s", msg.c_str());

    if (close(this->cfd) < 0) {
        fprintf(stderr, "ERROR: %s\n", strerror(errno));
    }
    if (close(this->cfd) < 0) {
        fprintf(stderr, "ERROR: %s\n", strerror(errno));
    }
    
    exit(0);
}

void Server::~Server() {
}
