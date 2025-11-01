CXX = g++
CXXFLAGS = -std=c++20 -Wall -Wextra -I./src -I./tests

TARGET = dns
TEST_BIN = tests/unit_tests

SRCS = src/arguments.cpp src/dns.cpp src/dnsmessage.cpp src/sock.cpp
TEST_SRC = tests/tests.cpp
TEST_SRCS = src/arguments.cpp src/dnsmessage.cpp src/sock.cpp

.PHONY: all run clean test

all: $(TARGET)

$(TARGET): $(SRCS)
	$(CXX) $(CXXFLAGS) $(SRCS) -o $(TARGET)

$(TEST_BIN): $(TEST_SRC) $(TEST_SRCS)
	$(CXX) $(CXXFLAGS) $(TEST_SRC) $(TEST_SRCS) -o $(TEST_BIN)

test: $(TEST_BIN)
	./$(TEST_BIN) -s

run: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(TARGET) $(TEST_BIN)
