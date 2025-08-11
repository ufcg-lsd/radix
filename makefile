# Compiler
CXX = g++

# Compiler flags
OPT_FLAGS = -std=c++17 -O3 -march=native -Wall
DEFAULT_FLAGS = -std=c++17 -Wall

# Targets
TARGET_OPT = radix_sort_opt
TARGET_NONOPT = radix_sort_non_opt

# Source file
SOURCE = radix_sort.cpp

# Build both
all: $(TARGET_OPT) $(TARGET_NONOPT)

# Optimized build
$(TARGET_OPT): $(SOURCE)
	$(CXX) $(OPT_FLAGS) -o $(TARGET_OPT) $(SOURCE)

# Non-optimized build
$(TARGET_NONOPT): $(SOURCE)
	$(CXX) $(DEFAULT_FLAGS) -o $(TARGET_NONOPT) $(SOURCE)

# Run optimized
run-opt: $(TARGET_OPT)
	./$(TARGET_OPT)

# Run non-optimized
run-nonopt: $(TARGET_NONOPT)
	./$(TARGET_NONOPT)

clean:
	rm -rf $(TARGET_OPT) $(TARGET_NONOPT)

