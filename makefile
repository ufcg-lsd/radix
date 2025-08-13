.PHONY: all clean build run_radix_sort run_radix_sort_opt

CXX = g++
CFLAGS = -std=c++17 -Wall
CFLAGS_OPT = -std=c++17 -O3 -march=native -Wall

SRC = src/radix_sort.cpp
TEST = tests/test_radix_sort.cpp

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

clean:
	rm -f tests/radix_sort tests/radix_sort_opt

