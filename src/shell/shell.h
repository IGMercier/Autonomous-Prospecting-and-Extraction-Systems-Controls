#ifndef _SHELL_H_
#define _SHELL_H_

extern char **environ;

class Shell {
    private:
        void evaluate(char *cmdline);
        int parseline(char *buf, char **argv);
        int command(char **argv);
        void listCommand();
    public:
        Shell();
        void run();
        ~Shell();
};


#endif
