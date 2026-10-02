CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -Wpedantic -Iinclude
TARGET = storageguard
SOURCES = src/main.cpp src/journal.cpp

all: $(TARGET)

$(TARGET): $(SOURCES) include/journal.h
	$(CXX) $(CXXFLAGS) $(SOURCES) -o $(TARGET)

clean:
	rm -f $(TARGET)

.PHONY: all clean