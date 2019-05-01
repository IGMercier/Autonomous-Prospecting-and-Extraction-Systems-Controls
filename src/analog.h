#ifndef ANALOG_H
#define ANALOG_H

class Component {
    public:
        Component();
        ~Component();
};

class Therm : public Component {
    private:
        int bus_addr;
        float max_T;
        int sample_freq;
        float iTemp;
    public:
        Therm(int bus_addr, float max_T, int sample_freq);
        ~Therm();
        float read_temp();
        float D_temp();
};

class Amm : public Component {
    private:
        int bus_addr;
        float max_I;
        int sample_freq;
    public:
        Amm(int bus_addr, float max_I, int sample_freq);
        ~Amm();
        float read_curr();
}

class Level : public Component {
    private:
        int bus_start;
        int bus_end;
        int sample_freq;
    public:
        Level(int bus_start, int bus_end, int sample_freq);
        ~Level();
        int read_level();
}

#endif
