CC = g++
TARGET_ALL = apes.out
TARGET_SERVER = server.out
TARGET_SHELL = shell.out

MISC_FILES = misc/rio.cpp misc/flags.cpp misc/flags_set.cpp
SERVER_FILES = tests/serverTest.cpp server/APESServer.cpp server/serverBase.cpp
SHELL_FILES = tests/shellTest.cpp shell/APESShell.cpp shell/shellBase.cpp
ALL_FILES = program.cpp APES.cpp components.cpp $(SHELL_FILES) $(SERVER_FILES)

CFLAGS = -g -Wall -fvisibility=hidden -std=c++14
INCL = -I/usr/include/python3.5m -I/libraries/pybind11/include/
PYFLAGS = `python3-config --cflags` $(INCL) `python3-config --ldflags`
LIBS = -lwiringPi -pthread

all: $(TARGET_ALL) $(TARGET_SERVER) $(TARGET_SHELL)

$(TARGET_SHELL): $(SHELL_FILES) $(MISC_FILES)
	$(CC) -o $(TARGET_SHELL) $(SHELL_FILES) $(MISC_FILES) $(CFLAGS) $(LIBS)

$(TARGET_SERVER): $(SERVER_FILES) $(MISC_FILES)
	$(CC) -o $(TARGET_SERVER) $(SERVER_FILES) $(MISC_FILES) $(CFLAGS) $(LIBS)

$(TARGET_ALL): $(ALL_FILES) $(MISC_FILES)
	$(CC) -o $(TARGET_ALL) $(ALL_FILES) $(MISC_FILES) $(CFLAGS) $(PYFLAGS) $(LIBS)

clean:
	$(RM) $(TARGET_ALL) $(TARGET_SERVER) $(TARGET_SHELL)
