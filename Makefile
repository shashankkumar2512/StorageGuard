CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -Wpedantic -Iinclude

TARGET = storageguard
TEST_TARGET = test_journal
SOURCES = src/main.cpp src/journal.cpp
TEST_SOURCES = tests/test_journal.cpp src/journal.cpp

all: $(TARGET)

$(TARGET): $(SOURCES) include/journal.h
	$(CXX) $(CXXFLAGS) $(SOURCES) -o $(TARGET)

$(TEST_TARGET): $(TEST_SOURCES) include/journal.h
	$(CXX) $(CXXFLAGS) $(TEST_SOURCES) -o $(TEST_TARGET)

test: $(TEST_TARGET)
	./$(TEST_TARGET)

clean:
	rm -f $(TARGET) $(TEST_TARGET)

.PHONY: all test clean