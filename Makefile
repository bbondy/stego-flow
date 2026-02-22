CXX := c++
CXXFLAGS := -std=c++17 -Wall -Wextra -Wpedantic -O2

TARGET := stego_demo
TEST_TARGET := stego_tests

IMAGE_FLOW_DIR := image-flow

# Build against image-flow sources directly through the git dependency.
IMAGE_FLOW_SRCS := \
	$(IMAGE_FLOW_DIR)/api.cpp \
	$(IMAGE_FLOW_DIR)/bmp.cpp \
	$(IMAGE_FLOW_DIR)/drawable.cpp \
	$(IMAGE_FLOW_DIR)/gif.cpp \
	$(IMAGE_FLOW_DIR)/jpg.cpp \
	$(IMAGE_FLOW_DIR)/layer.cpp \
	$(IMAGE_FLOW_DIR)/png.cpp

APP_SRCS := src/main.cpp src/steganography.cpp $(IMAGE_FLOW_SRCS)
TEST_SRCS := src/tests.cpp src/steganography.cpp $(IMAGE_FLOW_SRCS)

APP_OBJS := $(APP_SRCS:.cpp=.o)
TEST_OBJS := $(TEST_SRCS:.cpp=.o)

all: $(TARGET)

$(TARGET): $(APP_OBJS)
	$(CXX) $(CXXFLAGS) -I$(IMAGE_FLOW_DIR) -Isrc -o $@ $(APP_OBJS)

test: $(TEST_TARGET)
	./$(TEST_TARGET)

$(TEST_TARGET): $(TEST_OBJS)
	$(CXX) $(CXXFLAGS) -I$(IMAGE_FLOW_DIR) -Isrc -o $@ $(TEST_OBJS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -I$(IMAGE_FLOW_DIR) -Isrc -c $< -o $@

clean:
	rm -f $(APP_OBJS) $(TEST_OBJS) $(TARGET) $(TEST_TARGET) stego_message.png stego_test.png

.PHONY: all clean test
