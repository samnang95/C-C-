CXX ?= c++
CXXFLAGS ?= -std=c++17 -Wall -Wextra -Wpedantic
TARGET := task_manager
SOURCE := src/main.cpp

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(SOURCE)
	$(CXX) $(CXXFLAGS) $(SOURCE) -o $(TARGET)

clean:
	rm -f $(TARGET)
