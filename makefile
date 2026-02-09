.PHONY: all clean build radix_sort radix_sort_opt test clean_test

CXX = g++
CFLAGS = -std=c++17 -Wall
CFLAGS_OPT = -std=c++17 -O3 -march=native -Wall

SRC = src/radix_sort.cpp
BENCHMARK_SRC = tests/benchmark.cpp 

BUILD_DIR = build

BIN_RADIX = tests/radix_sort
BIN_RADIX_OPT = tests/radix_sort_opt

all: build

build: $(BIN_RADIX) $(BIN_RADIX_OPT)

$(BIN_RADIX): $(SRC) $(BENCHMARK_SRC)
	$(CXX) $(CFLAGS) $^ -o $@

$(BIN_RADIX_OPT): $(SRC) $(BENCHMARK_SRC)
	$(CXX) $(CFLAGS_OPT) $^ -o $@

# Benchmarks
radix_sort: $(BIN_RADIX)
	./$(BIN_RADIX)

radix_sort_opt: $(BIN_RADIX_OPT)
	./$(BIN_RADIX_OPT)

# CMake Wrapper
test:
	@echo "Configuring CMake..."
	@mkdir -p $(BUILD_DIR)
	@cd $(BUILD_DIR) && cmake ..
	@echo "Compiling unit tests..."
	@cd $(BUILD_DIR) && make
	@echo "Executing unit tests..."
	@./$(BUILD_DIR)/run_tests

clean:
	rm -f $(BIN_RADIX) $(BIN_RADIX_OPT)
	rm -rf $(BUILD_DIR)
	rm -f run_tests 