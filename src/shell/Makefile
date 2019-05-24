CC = g++
TARGET = shell
CFLAGS = -g -Wall -std=c++14
FILES = program.cpp shellBase.cpp  APESShell.cpp ../misc/rio.cpp
LIBS = -lpthread

all: $(TARGET)

$(TARGET): $(FILES)
	$(CC) -o $(TARGET) $(CFLAGS) $(FILES) $(LIBS)

clean:
	$(RM) -rf $(TARGET) $(TARGET).dSYM/
