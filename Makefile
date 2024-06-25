CXX := g++
CXXFLAGS := -Wall -Wextra -std=c++17
SRCS := main.cpp
OBJS := $(SRCS:.cpp=.o)

TARGET := main

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(OBJS) -o $(TARGET)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)

