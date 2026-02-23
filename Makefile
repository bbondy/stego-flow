CXX := c++
CXXFLAGS := -std=c++17 -Wall -Wextra -Wpedantic -O2

ARCH := $(shell uname -m)
BIN_DIR := build/bin
TARGET := $(BIN_DIR)/stego_flow
TEST_TARGET := $(BIN_DIR)/stego_tests
OBJ_DIR := build/intermediate/$(ARCH)

IMAGE_FLOW_DIR := image-flow
IMAGE_FLOW_SRC_DIR := $(IMAGE_FLOW_DIR)/src

# Build against image-flow sources directly through the git dependency.
IMAGE_FLOW_SRCS := \
	$(IMAGE_FLOW_SRC_DIR)/example_api.cpp \
	$(IMAGE_FLOW_SRC_DIR)/bmp.cpp \
	$(IMAGE_FLOW_SRC_DIR)/drawable.cpp \
	$(IMAGE_FLOW_SRC_DIR)/gif.cpp \
	$(IMAGE_FLOW_SRC_DIR)/jpg.cpp \
	$(IMAGE_FLOW_SRC_DIR)/layer.cpp \
	$(IMAGE_FLOW_SRC_DIR)/png.cpp \
	$(IMAGE_FLOW_SRC_DIR)/svg.cpp \
	$(IMAGE_FLOW_SRC_DIR)/webp.cpp

APP_SRCS := src/main.cpp src/steganography.cpp $(IMAGE_FLOW_SRCS)
TEST_SRCS := src/tests.cpp src/steganography.cpp $(IMAGE_FLOW_SRCS)

APP_OBJS := $(patsubst %.cpp,$(OBJ_DIR)/%.o,$(APP_SRCS))
TEST_OBJS := $(patsubst %.cpp,$(OBJ_DIR)/%.o,$(TEST_SRCS))

all: $(TARGET)

$(TARGET): $(APP_OBJS)
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -I$(IMAGE_FLOW_SRC_DIR) -Isrc -o $@ $(APP_OBJS)

test: $(TEST_TARGET)
	./$(TEST_TARGET)

$(TEST_TARGET): $(TEST_OBJS)
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -I$(IMAGE_FLOW_SRC_DIR) -Isrc -o $@ $(TEST_OBJS)

run: $(TARGET)
	./$(TARGET)

$(OBJ_DIR)/%.o: %.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -I$(IMAGE_FLOW_SRC_DIR) -Isrc -c $< -o $@

clean:
	rm -rf build $(TARGET) $(TEST_TARGET)

.PHONY: all clean test run
