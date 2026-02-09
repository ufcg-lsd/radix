cmake_minimum_required(VERSION 3.14)

project(RadixSortTCC)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/heads/main.zip
)
set(INSTALL_GTEST OFF)
FetchContent_MakeAvailable(googletest)

include_directories(src)

add_executable(
  run_tests
  tests/radix_sort_tests.cpp
  src/radix_sort.cpp
)

target_link_libraries(
  run_tests
  GTest::gtest_main
  pthread
)

include(GoogleTest)
gtest_discover_tests(run_tests)