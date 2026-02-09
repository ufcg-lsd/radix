.PHONY: all clean build run_radix_sort run_radix_sort_opt

CXX = g++
CFLAGS = -std=c++17 -Wall
CFLAGS_OPT = -std=c++17 -O3 -march=native -Wall

SRC = src/radix_sort.cpp
TEST = tests/benchmark.cpp

BUILD_TARGETS = tests/radix_sort tests/radix_sort_opt

all: build

build: $(BUILD_TARGETS)

tests/radix_sort: $(SRC) $(TEST)
	$(CXX) $(CFLAGS) $^ -o $@

tests/radix_sort_opt: $(SRC) $(TEST)
	$(CXX) $(CFLAGS_OPT) $^ -o $@

radix_sort:
	./tests/radix_sort

radix_sort_opt:
	./tests/radix_sort_opt

test:
	g++ tests/radix_sort_tests.cpp src/radix_sort.cpp -o run_tests -lgtest -lgtest_main -pthread -I src/
	./run_tests

clean:
	rm -f tests/radix_sort tests/radix_sort_opt

