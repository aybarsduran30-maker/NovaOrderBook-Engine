cmake_minimum_required(VERSION 3.16)
project(NovaOrderBookEngine CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)

include_directories(include)

add_library(nova_core STATIC
    src/OrderBook.cpp
)
target_include_directories(nova_core PUBLIC include)

FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG        v1.14.0
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
set(BUILD_GMOCK OFF CACHE BOOL "" FORCE)
set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

enable_testing()

add_executable(nova_tests tests/test_orderbook.cpp)
target_link_libraries(nova_tests PRIVATE nova_core GTest::gtest_main)
target_include_directories(nova_tests PRIVATE include)

include(GoogleTest)
gtest_discover_tests(nova_tests)

add_executable(nova_cli src/main.cpp)
target_link_libraries(nova_cli PRIVATE nova_core)
target_include_directories(nova_cli PRIVATE include)

FetchContent_Declare(
  pybind11
  GIT_REPOSITORY https://github.com/pybind/pybind11.git
  GIT_TAG        v2.12.0
)
FetchContent_MakeAvailable(pybind11)

pybind11_add_module(nova_engine src/bindings.cpp)
target_link_libraries(nova_engine PRIVATE nova_core)
target_include_directories(nova_engine PRIVATE include)

add_executable(nova_benchmark tests/benchmark.cpp)
target_link_libraries(nova_benchmark PRIVATE nova_core)
target_include_directories(nova_benchmark PRIVATE include)