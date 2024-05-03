cmake_minimum_required(VERSION 3.10)
add_library(fpga fpga.h xcl2.hpp Host.hpp fpga_version.cpp xcl2.cpp)
add_executable(fpga_test main.cpp)
target_link_libraries(fpga PUBLIC OpenCL pthread rt)
target_link_libraries(fpga_test fpga)
target_compile_features(fpga PRIVATE cxx_std_17)
target_include_directories(fpga
    PRIVATE $<BUILD_INTERFACE:${HEADER_DIR}>
    PRIVATE "${XILINX_XRT}/include"
    PRIVATE "${XILINX_VIVADO}/include"
)
