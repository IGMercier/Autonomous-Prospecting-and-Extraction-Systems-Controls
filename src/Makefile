CC = g++
TARGET = apes
CFLAGS = -g -Wall -fvisibility=hidden `python3-config --cflags`
LFLAGS = `python3-config --ldflags`
MAIN_FILES = program.cpp commands.cpp
SERVER_FILES = server/server.cpp server/serverBase.cpp server/rio.cpp
SHELL_FILES = shell/shell.cpp shell/helper.cpp
APES_FILES = APES.cpp components.cpp
LIBS = -lwiringPi -lpthread
INCL = -I/usr/include/python3.5m -I/libraries/pybind11/include/

all: $(TARGET)

$(TARGET): $(FILES)
	$(CC) -o $(TARGET) $(MAIN_FILES) $(SERVER_FILES) $(LIBS) $(CFLAGS) $(INCL) $(LFLAGS)

clean:
	$(RM) $(TARGET)
