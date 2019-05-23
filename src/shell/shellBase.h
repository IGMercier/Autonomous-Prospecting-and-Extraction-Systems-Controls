#ifndef _SHELL_BASE_H_
#define _SHELL_BASE_H_

extern char **environ;

#define MAXLINE 1024
#define MAXARGS 128

class ShellBase {
    protected:
        int *readFrom;
        void evaluate(char *cmdline);
        int parseline(char *buf, char **argv);
        int command(char **argv);
        void shell_print(char *msg);
    public:
        ShellBase(int *readFrom);
        void run(int *shell_cfd);
        ~ShellBase();
};


#endif
