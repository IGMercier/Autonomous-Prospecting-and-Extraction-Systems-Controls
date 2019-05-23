CC = g++
TARGET = server
CFLAGS = -g -Wall -std=c++14 -fvisibility=hidden
FILES = program.cpp server.cpp serverBase.cpp rio.cpp commands.cpp
LIBS = -pthread

all: $(TARGET)

$(TARGET): $(FILES)
	$(CC) -o $(TARGET) $(CFLAGS) $(FILES) $(LIBS)

clean:
	$(RM) $(TARGET)
