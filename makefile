CXX = g++
CC  = gcc
CXXFLAGS = -std=c++11 -Wall
CFLAGS   = -std=c11 -Wall
LDFLAGS = -lncurses
TARGET = cliwave

SRC_CPP = src/cliwave.cpp src/session.cpp
SRC_C   = src/audiomanager.c src/dependencies/miniaudio.c

OBJ_CPP = $(SRC_CPP:.cpp=.o)
OBJ_C   = $(SRC_C:.c=.o)
OBJ     = $(OBJ_CPP) $(OBJ_C)

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CXX) -o $(TARGET) $(OBJ) $(LDFLAGS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(TARGET) $(OBJ)

run: $(TARGET)
	./$(TARGET)

.PHONY: all clean run