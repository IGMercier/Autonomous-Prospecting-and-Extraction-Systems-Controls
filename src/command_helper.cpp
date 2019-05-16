#include <string>

const char* listCommands() {
    string msg = "Help - Commands:\n";
    msg += "help - prints this message\n";
    msg += "start - initializes system\n";
    msg += "standby - stops all components, system in standby\n";
    msg += "temp - returns current temperature reading\n";
    msg += "dtemp - returns temperature difference from startup\n";
    msg += "curr - returns current current reading\n";
    msg += "level - returns current level reading\n";
    msg += "wob - returns current force reading\n";
    msg += "motor_drive - drive the motor\n";
    msg += "motor_stop - stops the motor\n";
    msg += "drill_run - runs the drill\n";
    msg += "drill_stop - stops the drill\n";
    msg += "drill_cycle - changes drill pwm\n";
    msg += "auto - puts system in auto mode\n";
    msg += "quit - shutdown the entire system, including server\n";
    return msg.c_str();
}
