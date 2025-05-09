CXX = g++
CXXFLAGS = -Wall -Wextra -std=c++17 -O3

TARGET = rcgreedy_simulation
SRCS = main.cpp equi.cpp event_generator.cpp rcgreedy_base.cpp

all: $(TARGET)

$(TARGET): $(SRCS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SRCS)

clean:
	rm -f $(TARGET)