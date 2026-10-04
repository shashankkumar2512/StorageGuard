
CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -Wpedantic -Iinclude

TARGET = storageguard
TEST_TARGET = test_journal
RECOVERY_TEST = test_recovery
CRASH_TEST = test_crash_recovery
DRIVER_CLIENT = driver_client
DRIVER_FAILURE_TEST = test_driver_failure
CONCURRENT_TEST = test_concurrent_journal

SOURCES = src/main.cpp src/journal.cpp src/recovery.cpp src/driver_client.cpp
TEST_SOURCES = tests/test_journal.cpp src/journal.cpp src/driver_client.cpp
RECOVERY_SOURCES = tests/test_recovery.cpp src/journal.cpp src/recovery.cpp src/driver_client.cpp

all: $(TARGET)

$(TARGET): $(SOURCES) include/journal.h include/recovery.h include/driver_client.h
	$(CXX) $(CXXFLAGS) $(SOURCES) -o $(TARGET)

$(TEST_TARGET): $(TEST_SOURCES) include/journal.h include/driver_client.h
	$(CXX) $(CXXFLAGS) $(TEST_SOURCES) -o $(TEST_TARGET)

$(RECOVERY_TEST): $(RECOVERY_SOURCES) include/journal.h include/recovery.h include/driver_client.h
	$(CXX) $(CXXFLAGS) $(RECOVERY_SOURCES) -o $(RECOVERY_TEST)

$(CRASH_TEST): tests/test_crash_recovery.cpp $(TARGET)
	$(CXX) $(CXXFLAGS) tests/test_crash_recovery.cpp -o $(CRASH_TEST)

$(DRIVER_CLIENT): tests/test_driver_client.cpp src/driver_client.cpp include/driver_client.h
	$(CXX) $(CXXFLAGS) tests/test_driver_client.cpp src/driver_client.cpp -o $(DRIVER_CLIENT)

$(DRIVER_FAILURE_TEST): tests/test_driver_failure.cpp src/driver_client.cpp include/driver_client.h
	$(CXX) $(CXXFLAGS) tests/test_driver_failure.cpp src/driver_client.cpp -o $(DRIVER_FAILURE_TEST)

$(CONCURRENT_TEST): tests/test_concurrent_journal.cpp src/journal.cpp src/driver_client.cpp include/journal.h include/driver_client.h
	$(CXX) $(CXXFLAGS) tests/test_concurrent_journal.cpp src/journal.cpp src/driver_client.cpp -o $(CONCURRENT_TEST)

test: $(TEST_TARGET) $(RECOVERY_TEST) $(CRASH_TEST) $(DRIVER_FAILURE_TEST) $(CONCURRENT_TEST)
	./$(TEST_TARGET)
	./$(RECOVERY_TEST)
	./$(CRASH_TEST)
	./$(DRIVER_FAILURE_TEST)
	./$(CONCURRENT_TEST)

driver-test: $(DRIVER_CLIENT)
	./$(DRIVER_CLIENT)

clean:
	rm -f $(TARGET) $(TEST_TARGET) $(RECOVERY_TEST) $(CRASH_TEST) $(DRIVER_CLIENT) $(DRIVER_FAILURE_TEST) $(CONCURRENT_TEST)

.PHONY: all test driver-test clean
