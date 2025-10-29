CXX = g++
CXXFLAGS = -std=c++20 -Wall -Wextra -I./src -I./tests -DDEBUG_PRINT

TARGET = dns
TEST_BIN = tests/unit_tests

SRCS = $(wildcard src/*.cpp)
TEST_SRC = tests/tests.cpp
TEST_SRCS = $(filter-out src/dns.cpp, $(SRCS))

.PHONY: all run clean test

all: $(TARGET)

$(TARGET): $(SRCS)
	$(CXX) $(CXXFLAGS) $(SRCS) -o $(TARGET)

$(TEST_BIN): $(TEST_SRC) $(TEST_SRCS)
	$(CXX) $(CXXFLAGS) $(TEST_SRC) $(TEST_SRCS) -o $(TEST_BIN)

test: $(TEST_BIN)
	./$(TEST_BIN)

run: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(TARGET) $(TEST_BIN)
