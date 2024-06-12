CXX = g++
CXXFLAGS = -Wall -Wextra -std=c++20

TARGET = ./build/spinlock.out
SRCS = ./src/main.cpp

BUILD_DIR = ./build
OBJS = $(SRCS:./src/%.cpp=$(BUILD_DIR)/%.o)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJS)

$(BUILD_DIR)/%.o: ./src/%.cpp | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

clean:
	rm -rf $(BUILD_DIR)

.PHONY: all clean
