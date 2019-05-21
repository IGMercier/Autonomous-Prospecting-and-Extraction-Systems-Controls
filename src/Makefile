CC = g++
TARGET = apes
CFLAGS = -g -Wall -fvisibility=hidden `python3-config --cflags`
LFLAGS = `python3-config --ldflags`
MAIN_FILES = program.cpp helper.cpp components.cpp APES.cpp
SERVER_FILE = server/server.cpp server/serverBase.cpp
LIBS = -lwiringPi -lpthread
INCL = -I/usr/include/python3.5m -I/libraries/pybind11/include/

all: $(TARGET)

$(TARGET): $(FILES)
	$(CC) -o $(TARGET) $(MAIN_FILES) $(SERVER_FILES) $(LIBS) $(CFLAGS) $(INCL) $(LFLAGS)

clean:
	$(RM) $(TARGET)
