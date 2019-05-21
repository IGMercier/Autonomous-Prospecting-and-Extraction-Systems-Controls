CC = g++
TARGET = apes
CFLAGS = -g -Wall
FILES = server.cpp helper.cpp components.cpp APES.cpp
LIBS = -lwiringPi -lpthread
INCL = -I/usr/include/python3.5m -I/libraries/pybind11/include/

all: $(TARGET)

$(TARGET): $(FILES)
	$(CC) -o $(TARGET) $(FILES) $(LIBS) `python3-config --cflags` $(INCL) `python3-config --ldflags`  -fvisibility=hidden

clean:
	$(RM) $(TARGET)
