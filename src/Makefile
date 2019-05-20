CC = g++
TARGET = apes
CFLAGS = -g -Wall
FILES = server.cpp commands.cpp components.cpp APES.cpp
LIBS = -lpython3.5 -lwiringPi -lpthread -I/usr/include/python3.5m

all: $(TARGET)

$(TARGET): $(FILES)
	$(CC) -o $(TARGET) $(FILES) $(LIBS) -fvisibility=hidden

clean:
	$(RM) $(TARGET)
