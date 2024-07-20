CXX := g++
CXXFLAGS := -Wall -Wextra -std=c++20

SRC_DIR := ./src
OBJ_DIR := ./obj
BIN_DIR := ./bin

SRC_FILES := $(wildcard $(SRC_DIR)/*.cpp)

OBJ_FILES := $(SRC_FILES:$(SRC_DIR)/%.cpp=$(OBJ_DIR)/%.o)

TARGET := $(BIN_DIR)/swoosh

LIBS := -lssl -lcrypto

all: $(TARGET)

$(TARGET): $(OBJ_FILES)
	@mkdir -p $(BIN_DIR)
	$(CXX) $(OBJ_FILES) -o $@ $(LIBS)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -rf $(OBJ_DIR) $(BIN_DIR)

.PHONY: all clean
