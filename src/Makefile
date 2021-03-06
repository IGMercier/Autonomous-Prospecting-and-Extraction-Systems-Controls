CC = gcc
TARGET_ALL = apes.out
TARGET_SERVER = server.out
TARGET_SHELL = shell.out
TARGET_COMP = comp.out

SERVER_FILES = server/APESServer.cpp server/serverBase.cpp
SHELL_FILES = shell/APESShell.cpp shell/shellBase.cpp
COMP_FILES = APESsys/components.cpp
ALL_FILES = APESsys/APES.cpp $(COMP_FILES) $(SERVER_FILES) $(SHELL_FILES)

SERVER_TEST = tests/serverTest.cpp
SHELL_TEST = tests/shellTest.cpp
COMP_TEST = tests/componentsTest.cpp
ALL_TEST = program.cpp

CFLAGS = -g -Wall -fvisibility=hidden -std=c++14
INCL = -I/usr/include/python3.5m -I/libraries/pybind11/include/
PYFLAGS = `python3-config --cflags` $(INCL) `python3-config --ldflags`
LIBS = -lwiringPi -pthread -lstdc++

all: $(TARGET_ALL) $(TARGET_SERVER) $(TARGET_SHELL)

$(TARGET_COMP): $(COMP_TEST) $(COMP_FILES)
	$(CC) -o $(TARGET_COMP) $(COMP_TEST) $(COMP_FILES) $(CFLAGS) $(LIBS) $(PYFLAGS)

$(TARGET_SHELL): $(SHELL_TEST) $(SHELL_FILES)
	$(CC) -o $(TARGET_SHELL) $(SHELL_TEST) $(SHELL_FILES) $(CFLAGS) $(LIBS)

$(TARGET_SERVER): $(SERVER_TEST) $(SERVER_FILES)
	$(CC) -o $(TARGET_SERVER) $(SERVER_TEST) $(SERVER_FILES) $(CFLAGS) $(LIBS)

$(TARGET_ALL): $(ALL_TEST) $(ALL_FILES)
	$(CC) -o $(TARGET_ALL) $(ALL_TEST) $(ALL_FILES) $(CFLAGS) $(LIBS) $(PYFLAGS)

clean:
	$(RM) $(TARGET_ALL) $(TARGET_SERVER) $(TARGET_SHELL)
