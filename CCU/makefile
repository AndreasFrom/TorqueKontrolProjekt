# Compiler and flags
CXX = g++
CXXFLAGS = -Wall -std=c++17

# Files
SRC = filtertester_main.cpp src/filter.cpp  # Add other .cpp files here
OBJ = $(SRC:.cpp=.o)
EXE = my_program  # Name of the output executable

# Default target
all: $(EXE)

# Linking step to create the executable
$(EXE): $(OBJ)
	$(CXX) $(OBJ) -o $(EXE)

# Compiling the source files to object files
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Clean up object files and executable
clean:
	rm -f $(OBJ) $(EXE)

.PHONY: all clean
