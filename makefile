CXX = g++
CXXFLAGS = -std=c++11 -Wall
LDFLAGS = -lncurses
TARGET = cliwave
SRC = src/cliwave.cpp src/session.cpp

all: $(TARGET)

$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SRC) $(LDFLAGS)

clean:
	rm -f $(TARGET)

run: $(TARGET)
	./$(TARGET)

.PHONY: all clean run