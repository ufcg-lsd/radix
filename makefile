.PHONY: all clean run_bench test

CXX = g++
INCLUDES = -I./src
CXXFLAGS = -std=c++17 -O3 -march=native -Wall $(INCLUDES)

APP_BIN = sorter
APP_SRC = src/main.cpp src/external_sort.cpp src/radix_sort.cpp

BENCH_BIN = tests/radix_benchmark
BENCH_SRC = tests/benchmark.cpp src/radix_sort.cpp

BUILD_DIR = build

all: $(APP_BIN) $(BENCH_BIN)

$(APP_BIN): $(APP_SRC)
	$(CXX) $(CXXFLAGS) $(APP_SRC) -o $@

$(BENCH_BIN): $(BENCH_SRC)
	$(CXX) $(CXXFLAGS) $(BENCH_SRC) -o $@

run_bench_old: $(BENCH_BIN)
	./$(BENCH_BIN)

test:
	@mkdir -p $(BUILD_DIR)
	@cd $(BUILD_DIR) && cmake ..
	@cd $(BUILD_DIR) && make -j$(shell nproc)
	@./$(BUILD_DIR)/run_tests

clean:
	rm -f $(APP_BIN) $(BENCH_BIN)
	rm -rf $(BUILD_DIR)
	rm -f *.bin *.dat *.png results/*.png results/*.csv