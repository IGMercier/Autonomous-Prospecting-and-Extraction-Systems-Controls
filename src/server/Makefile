CC = g++
TARGET = server
CFLAGS = -g -Wall -fvisibility=hidden
FILES = program.cpp APESServer.cpp serverBase.cpp rio.cpp commands.cpp
LIBS = -lpthread

all: $(TARGET)

$(TARGET): $(FILES)
	$(CC) -o $(TARGET) $(CFLAGS) $(FILES) $(LIBS)

clean:
	$(RM) $(TARGET)
