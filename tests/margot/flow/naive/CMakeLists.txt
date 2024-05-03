cmake_minimum_required(VERSION 3.10)
add_library(naive naive.h naive.cpp)
target_include_directories(naive
    PRIVATE $<BUILD_INTERFACE:${HEADER_DIR}>
)
