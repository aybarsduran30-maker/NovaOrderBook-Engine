cmake_minimum_required(VERSION 3.16)
project(NovaOrderBookEngine VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(MSVC)
    add_compile_options(/O2 /W4 /permissive-)
else()
    add_compile_options(-O3 -march=native -Wall -Wextra -Wpedantic)
endif()

include_directories(include)

add_executable(nova_engine
    src/main.cpp
    src/OrderBook.cpp
)