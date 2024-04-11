cmake_minimum_required(VERSION 3.10)

project(map_matching)

file(GLOB srcs mma/*.cpp)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(mma etl/include libbambu/ac_types/include libbambu/ac_math/include)
add_definitions(-DDEBUG_PRINT)

add_executable(mma_xample ${srcs})

add_executable(mma_static ${srcs})
target_compile_definitions(mma_static PRIVATE -DSTATIC)