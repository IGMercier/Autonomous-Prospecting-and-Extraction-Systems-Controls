#ifndef _COMPONENTS_H_
#define _COMPONENTS_H_

#include "../libraries/pybind11/include/pybind11/embed.h"
#include <wiringPi.h>
#include <wiringPiSPI.h>
#include <wiringPiI2C.h>
#include <softPwm.h>

#include <linux/i2c-dev.h>
#include <sys/ioctl.h>

#define PWM_RANGE 200
#define PWM_MAX_FREQ 19.2e6

class Therm {
    private:
        int bus;
        int chan;
	    float iTemp;
    public:
        Therm(int bus, int chan);
        ~Therm();
        float read_temp();
	    float D_temp();
};

class Amm {
    private:
        int bus;
        int chan;
    public:
        Amm(int bus, int chan);
        ~Amm();
        float read_curr();
};

class WLevel {
    private:
        int bus;
        int chan_start;
        int chan_end;
    public:
        WLevel(int bus, int chan_start, int chan_end);
        ~WLevel();
        int read_wlevel();
};

class Wob {
    private:
        pybind11::object HX711;
    public:
        Wob(int pinA, int pinB);
        ~Wob();
        float read_wob();
};

class Encoder {
    private:
        int fd;
        int ppr; // pulse per revolution
    public:
        Encoder(int fd, int ppr);
        void reset();
        unsigned int getPulse();
        float calcVel(int n, int t);
        ~Encoder();
};

class Drill {
    private:
        int en_pin;
        int pwm_pin;
    public:
        Drill(int en_pin, int pwm_pin);
        void drill_run(int dc, float freq);
        void drill_stop();
        void drill_cycle(int dc, int on_period, float freq);
        ~Drill();
};

class Stepper {
    private:
        int pwm_pin;
        int dir_pin;
    public:
        Stepper(int pwm_pin, int dir_pin);
        void stepper_drive(int step, float freq);
        void stepper_stop();
        ~Stepper();
};

class Motor {
    protected:
        int dir_pin;
        int pwm_pin;
    public:
        Motor(int dir_pin, int pwm_pin);
        ~Motor();
        void motor_drive(int dir, int dc);
        void motor_stop();
};

class Pump : public Motor {
    public:
        Pump(int dir_pin, int pwm_pin) : Motor(dir_pin, pwm_pin) {};
        void pump_drive(int dir, int dc) {
            Motor::motor_drive(dir, dc);
        }
        void pump_stop() {
            Motor::motor_stop();
        }
        ~Pump() {}
};

class Solenoid {
    private:
        int pin;
    public:
        Solenoid(int pin);
        void openValve();
        void closeValve();
        ~Solenoid();
};

class DCHeater {
    private:
        int pin;
    public:
        DCHeater(int pin);
        void turnOn();
        void turnOff();
        ~DCHeater();
};

class Relay {
    private:
        int pin;
    public:
        Relay(int pin);
        void turnOn();
        void turnOff();
        ~Relay();
};

#endif
