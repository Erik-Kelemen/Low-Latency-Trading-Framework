# Makefile for the Trading Software Engineering Project

# Compiler and compiler flags
CXX := g++
CXXFLAGS := -std=c++17 -Wall -Wextra

# Source files
SRCS := main.cpp web_scraper.cpp interpolator.cpp data_publisher.cpp \
        controller.cpp trading_engine.cpp position_calculator.cpp

# Object files (automatically derived from source files)
OBJS := $(SRCS:.cpp=.o)

# Output executable
EXECUTABLE := trading_app

# Rule to build the executable
$(EXECUTABLE): $(OBJS)
	$(CXX) $(CXXFLAGS) $(OBJS) -o $(EXECUTABLE)

# Rule to compile source files to object files
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Clean rule to remove object files and the executable
clean:
	rm -f $(OBJS) $(EXECUTABLE)